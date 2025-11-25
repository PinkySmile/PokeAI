import argparse
import json
import os
import sys
import warnings
import types
from pathlib import Path
import gymnasium as gym
import onnx
import onnxruntime as ort
import torch
import numpy as np

try:
    from train import Agent
except ImportError:
    sys.exit(1)


def validate_model(path, dummy_obs, dummy_mask, original_output=None):
    print(f"Validating model at {path}...")

    # Structure check
    try:
        model = onnx.load(path)
        onnx.checker.check_model(model)
        print(" - Structure check passed.")
    except Exception as e:
        print(f" - Structure check FAILED: {e}")
        return

    # Inference check
    try:
        sess = ort.InferenceSession(path, providers=['CPUExecutionProvider'])
        ort_outs = sess.run(None, {
            'observation': dummy_obs.numpy(),
            'action_mask': dummy_mask.numpy()
        })
        print(" - Inference check passed.")
    except Exception as e:
        print(f" - Inference check FAILED: {e}")
        return

    # Numerical check
    if original_output is not None:
        idx = 3
        if len(original_output) > idx and len(ort_outs) > idx:
            torch_val = original_output[idx].detach().numpy()
            ort_val = ort_outs[idx]
            try:
                np.testing.assert_allclose(torch_val, ort_val, rtol=1e-03, atol=1e-04)
                print(" - Numerical accuracy passed.")
            except AssertionError as e:
                print(f" ! Warning: Numerical mismatch: {e}")


def export(checkpoint_path, output_path=None, opset=14):
    if not os.path.exists(checkpoint_path):
        raise FileNotFoundError(f"Checkpoint not found: {checkpoint_path}")

    if output_path is None:
        output_path = str(Path(checkpoint_path).with_suffix('.onnx'))

    print(f"Loading checkpoint: {checkpoint_path}")
    checkpoint = torch.load(checkpoint_path, map_location='cpu')

    # Setup dummy environment
    input_obs_shape = (494,)
    input_action_n = 12

    class DummyEnv:
        single_observation_space = gym.spaces.Box(low=0, high=255, shape=input_obs_shape, dtype=int)
        single_action_space = gym.spaces.Discrete(input_action_n)

    envs = types.SimpleNamespace(
        single_observation_space=DummyEnv.single_observation_space,
        single_action_space=DummyEnv.single_action_space,
        close=lambda: None
    )

    agent = Agent(envs).to('cpu')

    # Load weights
    state_dict = checkpoint["model_state"]
    clean_state_dict = {k.replace('_orig_mod.', ''): v for k, v in state_dict.items()}

    try:
        agent.load_state_dict(clean_state_dict)
    except RuntimeError:
        print("Warning: Strict loading failed, retrying loosely...")
        agent.load_state_dict(clean_state_dict, strict=False)

    agent.eval()

    # Create dummy inputs
    dummy_obs = torch.randn(1, *input_obs_shape)
    dummy_mask = torch.ones(1, input_action_n, dtype=torch.bool)

    move_indices = [35, 37, 39, 41, 78, 80, 82, 84]
    dummy_obs[:, move_indices] = 1.0

    print(f"Exporting to {output_path} (Opset {opset})...")

    # Export
    with warnings.catch_warnings():
        warnings.filterwarnings("ignore", category=torch.jit.TracerWarning)

        with torch.no_grad():
            torch_out = agent(dummy_obs, dummy_mask)

            torch.onnx.export(
                agent,
                (dummy_obs, dummy_mask),
                output_path,
                dynamo=False,
                export_params=True,
                opset_version=opset,
                do_constant_folding=True,
                input_names=['observation', 'action_mask'],
                output_names=['action', 'log_prob', 'entropy', 'value'],
                dynamic_axes={
                    k: {0: 'batch_size'} for k in
                    ['observation', 'action_mask', 'action', 'log_prob', 'entropy', 'value']
                }
            )

    validate_model(output_path, dummy_obs, dummy_mask, torch_out)

    # Save metadata
    metadata = {
        "checkpoint_path": checkpoint_path,
        "opset_version": opset,
        "observation_shape": [int(x) for x in input_obs_shape],
        "action_space_n": int(input_action_n),
        "args": checkpoint.get('args', {})
    }

    meta_path = str(Path(output_path).with_suffix('.json'))
    with open(meta_path, 'w') as f:
        json.dump(metadata, f, indent=2)

    if os.path.exists(output_path):
        print(f"Success! Size: {os.path.getsize(output_path) / (1024 * 1024):.2f} MB")


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Export PyTorch RL Agent to ONNX')
    parser.add_argument('--checkpoint', type=str, required=True, help='Path to .pt checkpoint')
    parser.add_argument('--output', type=str, default=None, help='Output ONNX path')
    parser.add_argument('--opset', type=int, default=14, help='ONNX opset version')

    args = parser.parse_args()
    export(args.checkpoint, args.output, args.opset)