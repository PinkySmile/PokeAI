import time
import argparse
import torch
import torch.nn as nn
import torch.optim as optim
from torch.distributions.categorical import Categorical
from torch.utils.tensorboard import SummaryWriter
import random
import numpy as np
import gymnasium as gym
import os
import glob
import json

# --- PokeAI imports ---
from PokeBattle.Gen1.Env import Examples, basic_opponent
from PokeBattle.Gen1.Pokemon import PokemonSpecies
from PokeBattle.Gen1.Move import AvailableMove
from PokeBattle.Gen1.State import BattleAction, BattleState
from gymnasium import logger

# --- Load config paths ---
with open("ai/configs/config.json", "r") as f:
    CONFIG = json.load(f).get("paths", {})

LOGS_ROOT = CONFIG.get("logs_root")
VIDEOS_SUBDIR = CONFIG.get("videos_subdir")
RUNS_DIR = CONFIG.get("runs_dir")

def get_replay_folder(run_name: str):
    return f'{LOGS_ROOT}/{run_name}/{VIDEOS_SUBDIR}'

# --- Player and Opponent teams definition ---
START_OPTIONS = {
        "p1name": "PokeAI",
        "p2name": "Opponent",
        "p1team": [{
            "species": PokemonSpecies.Charmander,
            "name": "CHARMANDER",
            "level": 10,
            "moves": [AvailableMove.Bonemerang, AvailableMove.Water_Gun, AvailableMove.Thundershock]
        }],
        "p2team": [{
                "species": PokemonSpecies.Articuno,
                "name": "ARTICUNO",
                "level": 5,
                "moves": [AvailableMove.Tackle, AvailableMove.Tail_Whip]
            },
            {
                "species": PokemonSpecies.Diglett,
                "name": "DIGLETT",
                "level": 8,
                "moves": [AvailableMove.Tackle, AvailableMove.Tail_Whip]
            },
            {
                "species": PokemonSpecies.Ponyta,
                "name": "PONYTA",
                "level": 8,
                "moves": [AvailableMove.Tackle, AvailableMove.Tail_Whip]
            }
        ]
    }

# START_OPTIONS = Examples.LtSurge

def _capture_frame(self):
    assert self.recording, "Cannot capture a frame, recording wasn't started."

    frame = self.env.render()
    if isinstance(frame, list):
        if len(frame) == 0:  # render was called
            return
        self.render_history += frame
        self.recorded_frames += frame

    elif isinstance(frame, np.ndarray):
        self.recorded_frames.append(frame)
    else:
        self.stop_recording()
        logger.warn(
            f"Recording stopped: expected type of frame returned by render to be a numpy array, got instead {type(frame)}."
        )

gym.wrappers.RecordVideo._capture_frame = _capture_frame


def make_env(gym_id, seed, idx, capture_video, run_name, video_every):
    fn_episode_trigger = lambda ep_id: ep_id % max(1, int(video_every)) == 0
    def thunk():
        # Use rgb_array only for the video env. otherwise standard env
        if capture_video and idx == 0:
            if gym_id == 'PokemonYellow':
                env = gym.make('PokemonYellow', seed, render_mode='rgb_array_list', opponent_callback=basic_opponent, episode_trigger=fn_episode_trigger, replay_folder=get_replay_folder(run_name), shuffle_teams=True)
            else:
                env = gym.make(gym_id, seed, render_mode='rgb_array')
            # record every `video_every` episodes on env 0
            env = gym.wrappers.RecordVideo(
                env,
                get_replay_folder(run_name),
                episode_trigger=fn_episode_trigger
            )
        else:
            try:
                if gym_id == 'PokemonYellow':
                    env = gym.make('PokemonYellow', seed, opponent_callback=basic_opponent, render_mode='human' if idx==0 else None, replay_folder=get_replay_folder(run_name), shuffle_teams=True)
                else:
                    env = gym.make(gym_id, seed)
            except TypeError:
                env = gym.make(gym_id, seed, render_mode=None)
        env = gym.wrappers.RecordEpisodeStatistics(env)
        # seed action/obs spaces for reproducibility
        try:
            env.action_space.seed(seed)
            env.observation_space.seed(seed)
        except Exception:
            pass
        return env

    return thunk


def layer_init(layer, std=np.sqrt(2), bias_const=0.0):
    torch.nn.init.orthogonal_(layer.weight, std)
    torch.nn.init.constant_(layer.bias, bias_const)
    return layer


class Agent(nn.Module):
    def __init__(self, envs):
        super(Agent, self).__init__()
        obs_dim = int(np.array(envs.single_observation_space.shape).prod())
        act_dim = envs.single_action_space.n

        self.move_embedding = nn.Embedding(
            num_embeddings=166,  # 0-164 moves, 165: token
            embedding_dim=16,
            padding_idx=0
        )

        embedded_obs_dim = obs_dim - 8 + (8 * 16)

        # Shared feature extractor
        self.feature_extractor = nn.Sequential(
            layer_init(nn.Linear(embedded_obs_dim, 2048)),
            nn.Tanh(),
            layer_init(nn.Linear(2048, 1024)),
            nn.Tanh(),
        )

        # Project features to embedding dimension for move scoring
        self.move_scorer = layer_init(nn.Linear(1024, 16), std=0.01)

        # Separate heads for switch and other actions
        self.other_actions = layer_init(nn.Linear(1024, 8), std=0.01)

        self.critic = nn.Sequential(
            layer_init(nn.Linear(embedded_obs_dim, 2048)),
            nn.Tanh(),
            layer_init(nn.Linear(2048, 1024)),
            nn.Tanh(),
            layer_init(nn.Linear(1024, 1), std=1.),
        )

    def embed_observation(self, x):
        batch_size = x.shape[0]

        move_id_positions = [35, 37, 39, 41, 78, 80, 82, 84]
        move_ids = x[:, move_id_positions].long()
        move_ids = move_ids.clone()
        move_ids = torch.where(move_ids == -10, torch.zeros_like(move_ids), move_ids)
        move_ids = torch.where(move_ids == -1, torch.ones_like(move_ids) * 165, move_ids)

        move_embeds = self.move_embedding(move_ids)
        move_embeds = move_embeds.view(batch_size, -1)

        all_positions = list(range(x.shape[1]))
        non_move_positions = [i for i in all_positions if i not in move_id_positions]

        non_move_features = x[:, non_move_positions]
        return torch.cat([non_move_features, move_embeds], dim=1)

    def get_value(self, x):
        x_embedded = self.embed_observation(x)
        return self.critic(x_embedded)

    def get_action_and_value(self, x, mask=None, action=None):
        batch_size = x.shape[0]
        x_embedded = self.embed_observation(x)

        # Get move embeddings for current pokemon
        move_id_positions = [35, 37, 39, 41]
        move_ids = x[:, move_id_positions].long()
        move_ids = move_ids.clone()
        move_ids = torch.where(move_ids == -10, torch.zeros_like(move_ids), move_ids)
        move_ids = torch.where(move_ids == -1, torch.ones_like(move_ids) * 165, move_ids)
        move_embeds = self.move_embedding(move_ids)  # (batch, 4, 16)

        features = self.feature_extractor(x_embedded)  # (batch, 1024)

        # Project to embedding space and compute move values
        move_query = self.move_scorer(features)  # (batch, 16)
        move_values = torch.bmm(move_embeds, move_query.unsqueeze(-1)).squeeze(-1)  # (batch, 4)
        other_values = self.other_actions(features)  # (batch, 8)

        # given the current state features, what kind of move characteristics do I want? (the move_query), then scores each available move based on how well it matches those desired characteristic

        logits = torch.cat([move_values, other_values], dim=1)  # (batch, 12)

        if mask is not None:
            logits = logits.masked_fill(mask == 0, float('-inf'))

        invalid_logits_row = torch.isneginf(logits).all(dim=1)
        if invalid_logits_row.any():
            raise ValueError(f"No valid actions available. Check the action mask.")

        probs = Categorical(logits=logits)
        if action is None:
            action = probs.sample()

        return action, probs.log_prob(action), probs.entropy(), self.critic(x_embedded)

# class Agent(nn.Module):
#     def __init__(self, envs):
#         super(Agent, self).__init__()
#         obs_dim = int(np.array(envs.single_observation_space.shape).prod())
#         print(obs_dim)
#         act_dim = envs.single_action_space.n
#         self.network = nn.Sequential(
#
#         )
#         self.critic = nn.Sequential(
#             layer_init(nn.Linear(obs_dim, 1024)),
#             nn.Tanh(),
#             layer_init(nn.Linear(1024, 512)),
#             nn.Tanh(),
#             layer_init(nn.Linear(512, 1), std=1.),
#         )
#         self.actor = nn.Sequential(
#             layer_init(nn.Linear(obs_dim, 1024)),
#             nn.Tanh(),
#             layer_init(nn.Linear(1024, 512)),
#             nn.Tanh(),
#             layer_init(nn.Linear(512, act_dim), std=0.01),
#         )
#
#     def get_value(self, x):
#         return self.critic(x)
#
#     def get_action_and_value(self, x, mask=None, action=None):
#         logits = self.actor(x)
#         if mask is not None:
#             logits = logits.masked_fill(mask == 0, float('-inf'))
#             # print(logits)
#         probs = Categorical(logits=logits)
#         if action is None:
#             action = probs.sample()
#         # print(probs.probs)
#         return action, probs.log_prob(action), probs.entropy(), self.critic(x)

# # Share common layers
# class Agent(nn.Module):
#     def __init__(self, envs):
#         super(Agent, self).__init__()
#         obs_dim = int(np.array(envs.single_observation_space.shape).prod())
#         act_dim = envs.single_action_space.n
#
#         self.network = nn.Sequential(
#             layer_init(nn.Linear(obs_dim, 2048)),
#             nn.Tanh(),
#             layer_init(nn.Linear(2048, 1024)),
#             nn.Tanh(),
#         )
#
#         self.critic = layer_init(nn.Linear(1024, 1), std=1.)
#         self.actor = layer_init(nn.Linear(1024, act_dim), std=0.01)
#
#     def get_value(self, x):
#         features = self.network(x)
#         return self.critic(features)
#
#     def get_action_and_value(self, x, mask=None, action=None):
#         features = self.network(x)
#         logits = self.actor(features)
#         # print(f'Action mask: {mask}')
#         if mask is not None:
#             logits = logits.masked_fill(mask == 0, float('-inf'))
#         probs = Categorical(logits=logits)
#         if action is None:
#             action = probs.sample()
#         return action, probs.log_prob(action), probs.entropy(), self.critic(features)


#--- Parser ---
def parse_args():
    parser = argparse.ArgumentParser()

    parser.add_argument('--exp-name', type=str, default='exp', help='Name of the experiment')
    parser.add_argument('--gym-id', type=str, default='CartPole-v1', help='ID of the Gymnasium environment')
    parser.add_argument('--learning-rate', type=float, default=2.5e-4, help='Learning rate of the optimizer')
    parser.add_argument('--disable-anneal-lr', action='store_false', dest='anneal_lr', default=True, help='Toggle learning rate annealing for policy and value networks')
    parser.add_argument('--seed', type=int, default=42, help='Seed used for the training')
    parser.add_argument('--total-timesteps', type=int, default=500000, help='The total timesteps of the training')
    # For GPU
    parser.add_argument('--torch-deterministic', type=bool, default=True, help='torch.backends.cudnn.deterministic=True by default')
    parser.add_argument('--use-cpu', action='store_true', default=False, help='To use CPU only')
    # Logging
    parser.add_argument('--wandb', action='store_true', default=False, help='Log the experiment using wandb')
    parser.add_argument('--wandb-project-name', type=str, default='ppo', help='wandb project name')
    parser.add_argument('--capture-video', action='store_true', default=False, help='Save video of the agent in its environment')
    parser.add_argument('--video-every', type=int, default=100, help='Record a video every N episodes on env 0')

    parser.add_argument('--num-envs', type=int, default=16, help='Number of parallel environments to run')
    parser.add_argument('--num-steps', type=int, default=128, help='Number of steps to run in each environments per policy rollout')

    parser.add_argument('--gamma-gae',  type=float, default=0.995, help='The discount factor')
    parser.add_argument('--lambda-gae',  type=float, default=0.95, help='The lambda for GAE')
    parser.add_argument('--num-minibatches', type=int, default=8, help='The number of minibatches')
    parser.add_argument('--update_epochs', type=int, default=4, help='The number of epochs to update the policy')
    parser.add_argument('--no-norm-adv', action='store_false', dest='norm_adv', default=True, help='Do not normalize advantages')
    parser.add_argument('--clip-coef', type=float, default=0.2, help='The surrogate clipping coefficient')
    parser.add_argument('--no-clip-vloss', action='store_false', dest='clip_vloss', default=True, help='Using clipped loss for the value function by default')
    parser.add_argument('--ent-coef', type=float, default=0.01, help='coefficient for the entropy loss')
    parser.add_argument('--vf-coef', type=float, default=0.5, help='coefficient of the value function')
    parser.add_argument('--max-grad-norm', type=float, default=0.5, help='The maximum norm for the gradient clipping')
    parser.add_argument('--target-kl', type=float, default=None, help='the KL divergence threshold for early stopping')

    args = parser.parse_args()
    args.batch_size = int(args.num_envs * args.num_steps)
    args.minibatch_size = int(args.batch_size // args.num_minibatches)
    return args


if __name__ == '__main__':
    args = parse_args()
    run_name = f'{args.gym_id}_{args.exp_name}_{args.seed}_{int(time.time())}'

    try:
        run_videos_dir = os.path.join(LOGS_ROOT, run_name, VIDEOS_SUBDIR)
        run_wandb_dir = os.path.join(LOGS_ROOT, run_name)
        run_runs_dir = os.path.join(RUNS_DIR, run_name)

        os.makedirs(run_videos_dir, exist_ok=True)
        os.makedirs(run_wandb_dir, exist_ok=True)
        os.makedirs(run_runs_dir, exist_ok=True)
    except Exception:
        logger.warn("Could not create directories.")
        pass

    #--- Logging ---
    if args.wandb:
        import wandb
        wandb.init(
            project=args.wandb_project_name,
            sync_tensorboard=True,
            config=vars(args),
            name=run_name,
            save_code=True,
            dir=f'{LOGS_ROOT}/{run_name}'
        )
    writer = SummaryWriter(f'{RUNS_DIR}/{run_name}')
    writer.add_text('hyperparameters',
                    '|param|value|\n|-|-|\n%s' % ('\n'.join(f'|{key}|{value}|' for key, value in vars(args).items())))

    #--- Fixing the seed for reproducibility ---
    random.seed(args.seed)
    np.random.seed(args.seed)
    torch.manual_seed(args.seed)
    torch.backends.cudnn.deterministic = args.torch_deterministic

    # GPU / CPU
    device = torch.device('cuda' if torch.cuda.is_available() and not args.use_cpu else 'cpu')
    print(f'Using {device}')

    # Creating the vectorized environments to run multiple independent copies of the same environment in parallel
    envs = gym.vector.AsyncVectorEnv([make_env(args.gym_id, args.seed + i, i, args.capture_video, run_name, args.video_every) for i in range(args.num_envs)])
    assert isinstance(envs.single_action_space, gym.spaces.Discrete), 'We only support discrete action space'

    agent = Agent(envs).to(device)
    # try torch.compile for speedups (optional)
    try:
        if hasattr(torch, "compile"):
            agent = torch.compile(agent)
    except Exception:
        pass

    optimizer = optim.Adam(agent.parameters(), lr=args.learning_rate, eps=1e-5)

    #--- Training storage (preallocated on device) ---
    obs = torch.zeros((args.num_steps, args.num_envs) + envs.single_observation_space.shape, dtype=torch.float32, device=device) # Ex: shape [128, 4, 4]
    actions = torch.zeros((args.num_steps, args.num_envs) + envs.single_action_space.shape, dtype=torch.long, device=device) #Ex: shape [128, 4]
    logprobs = torch.zeros((args.num_steps, args.num_envs), dtype=torch.float32, device=device)
    rewards = torch.zeros((args.num_steps, args.num_envs), dtype=torch.float32, device=device)
    dones = torch.zeros((args.num_steps, args.num_envs), dtype=torch.float32, device=device)
    values = torch.zeros((args.num_steps, args.num_envs), dtype=torch.float32, device=device)
    masks = torch.zeros((args.num_steps, args.num_envs, envs.single_action_space.n),dtype=torch.bool, device=device)

    global_step = 0
    start_time = time.time()
    # next_obs, _ = envs.reset(seed=args.seed)
    next_obs, next_obs_info = envs.reset(seed=args.seed, options=START_OPTIONS)
    next_mask = torch.as_tensor(next_obs_info.get('mask'), device=device, dtype=torch.bool)
    next_obs = torch.as_tensor(next_obs, dtype=torch.float32, device=device)
    next_done = torch.zeros(args.num_envs, dtype=torch.float32, device=device)
    num_updates = args.total_timesteps // args.batch_size

    #--- Training loop ---
    for update in range(1, num_updates + 1):
        if args.anneal_lr:
            frac = 1.0 - (update - 1.0) / num_updates
            lrnow = frac * args.learning_rate
            optimizer.param_groups[0]['lr'] = lrnow

        for step in range(0, args.num_steps):
            global_step += args.num_envs
            obs[step] = next_obs
            dones[step] = next_done

            with torch.no_grad():
                action, logprob, _, value = agent.get_action_and_value(x=next_obs, mask=next_mask)
                values[step] = value.view(-1) # or values[step] = value.flatten() for non contiguous tensor
            actions[step] = action
            logprobs[step] = logprob

            next_obs, reward, terminated, truncated, infos = envs.step(action.cpu().numpy())
            next_mask = torch.as_tensor(infos.get('mask'), device=device, dtype=torch.bool)

            next_done = np.logical_or(terminated, truncated)

            rewards[step] = torch.as_tensor(reward, device=device, dtype=torch.float32).view(-1)
            masks[step] = next_mask

            next_obs = torch.as_tensor(next_obs, device=device, dtype=torch.float32)
            next_done = torch.as_tensor(next_done, device=device, dtype=torch.float32)

            ep_infos = infos.get('_episode')
            if "_episode" in infos:
                for i in range(len(infos["_episode"])):
                    if infos["_episode"][i]:
                        ep_return = infos["episode"]["r"][i]
                        ep_length = infos["episode"]["l"][i]
                        print(f"global_step={global_step}, env={i}, episode_lenght: {ep_length}, episodic_return={ep_return}")
                        writer.add_scalar("charts/episodic_return", ep_return, global_step)
                        writer.add_scalar("charts/episodic_length", ep_length, global_step)
                        # Log the latest recorded video to W&B when env 0 finishes an episode
                        if args.wandb and args.capture_video and i == 0:
                            try:
                                video_dir = os.path.join(get_replay_folder(run_name))
                                mp4s = sorted(glob.glob(os.path.join(video_dir, '*.mp4')))
                                if mp4s:
                                    wandb.log({"video/rollout": wandb.Video(mp4s[-1], format="mp4")})
                            except Exception:
                                pass

        #--- General Advantage Estimation (GAE) (once per rollout) ---
        with torch.no_grad():
            next_value = agent.get_value(next_obs).reshape(1, -1)
            advantages = torch.zeros_like(rewards, device=device)
            lastgaelam = 0.0
            for t in reversed(range(args.num_steps)):
                if t == args.num_steps - 1:
                    nextnonterminal = 1.0 - next_done
                    nextvalues = next_value
                else:
                    nextnonterminal = 1.0 - dones[t + 1]
                    nextvalues = values[t + 1]
                delta = rewards[t] + args.gamma_gae * nextvalues * nextnonterminal - values[t]
                lastgaelam = delta + args.gamma_gae * args.lambda_gae * nextnonterminal * lastgaelam
                advantages[t] = lastgaelam
            returns = advantages + values

        # Flatten the batch
        b_obs = obs.reshape((-1,) + envs.single_observation_space.shape)
        b_logprobs = logprobs.reshape(-1)
        b_actions = actions.reshape(-1)
        b_advantages = advantages.reshape(-1)
        b_returns = returns.reshape(-1)
        b_values = values.reshape(-1)
        b_masks = masks.reshape((-1, envs.single_action_space.n))

        # Optimizing the policy and value network
        batch_size = args.batch_size
        minibatch_size = args.minibatch_size
        clipfracs = []
        approx_kl = None
        for epoch in range(args.update_epochs):
            perm = torch.randperm(batch_size, device=device)
            for start in range(0, batch_size, minibatch_size):
                end = start + minibatch_size
                mb_inds = perm[start:end]
                _, newlogprob, entropy, newvalue = agent.get_action_and_value(b_obs[mb_inds],
                                                                              mask=b_masks[mb_inds], # A changer
                                                                              action=b_actions[mb_inds])

                logratio = newlogprob - b_logprobs[mb_inds]
                ratio = logratio.exp()

                # DEBUG: How aggressively the policy updates
                with torch.no_grad():
                    old_approx_kl = (-logratio).mean()
                    approx_kl = ((ratio - 1) - logratio).mean()
                    clipfracs += [((ratio - 1.0).abs() > args.clip_coef).float().mean().item()]

                # Minibatch normalization
                mb_advantages = b_advantages[mb_inds]
                if args.norm_adv:
                    mb_advantages = (mb_advantages - mb_advantages.mean()) / (mb_advantages.std() + 1e-8)

                # Policy loss
                pg_loss1 = -mb_advantages * ratio
                pg_loss2 = -mb_advantages * torch.clamp(ratio, 1 - args.clip_coef, 1 + args.clip_coef)
                pg_loss = torch.max(pg_loss1, pg_loss2).mean()

                # Value loss
                newvalue = newvalue.view(-1)
                if args.clip_vloss:
                    v_loss_unclipped = (newvalue - b_returns[mb_inds]) ** 2
                    v_clipped = b_values[mb_inds] + torch.clamp(
                        newvalue - b_values[mb_inds],
                        -args.clip_coef,
                        args.clip_coef,
                    )
                    v_loss_clipped = (v_clipped - b_returns[mb_inds]) ** 2
                    v_loss_max = torch.max(v_loss_unclipped, v_loss_clipped)
                    v_loss = 0.5 * v_loss_max.mean()
                else:
                    v_loss = 0.5 * ((newvalue - b_returns[mb_inds]) ** 2).mean()

                entropy_loss = entropy.mean()
                loss = pg_loss - args.ent_coef * entropy_loss + v_loss * args.vf_coef

                optimizer.zero_grad()
                loss.backward()
                nn.utils.clip_grad_norm_(agent.parameters(), args.max_grad_norm)
                optimizer.step()

            if args.target_kl is not None and approx_kl is not None and approx_kl > args.target_kl:
                break

        # DEBUG: Is the value function a good indicator of the returns ?
        y_pred, y_true = b_values.cpu().numpy(), b_returns.cpu().numpy()
        var_y = np.var(y_true)
        explained_var = np.nan if var_y == 0 else 1 - np.var(y_true - y_pred) / var_y

        # Logging
        writer.add_scalar("charts/learning_rate", optimizer.param_groups[0]["lr"], global_step)
        writer.add_scalar("losses/value_loss", v_loss.item(), global_step)
        writer.add_scalar("losses/policy_loss", pg_loss.item(), global_step)
        writer.add_scalar("losses/entropy", entropy_loss.item(), global_step)
        if 'old_approx_kl' in locals():
            writer.add_scalar("losses/old_approx_kl", old_approx_kl.item(), global_step)
        if approx_kl is not None:
            writer.add_scalar("losses/approx_kl", approx_kl.item(), global_step)
        writer.add_scalar("losses/clipfrac", np.mean(clipfracs), global_step)
        writer.add_scalar("losses/explained_variance", explained_var, global_step)
        print("SPS:", int(global_step / (time.time() - start_time)))
        writer.add_scalar("charts/SPS", int(global_step / (time.time() - start_time)), global_step)

    envs.close()
    writer.close()
