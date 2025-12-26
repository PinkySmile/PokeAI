import os
import sys
import subprocess
from argparse import ArgumentParser
from PokeBattle.Gen1.BattleHandler import BattleHandler
from PokeBattle.Gen1.Gen1Renderer import Gen1Renderer

renderers = {
	'yellow':           (Gen1Renderer, ("",   True)),
	'yellow_colorless': (Gen1Renderer, ("",   False)),
	'red':              (Gen1Renderer, ("r",  True)),
	'red_colorless':    (Gen1Renderer, ("r",  False)),
	'green':            (Gen1Renderer, ("rg", True)),
	'green_colorless':  (Gen1Renderer, ("rg", False)),
}

parser = ArgumentParser(prog=sys.argv[0])
parser.add_argument('-r', '--renderer', default='yellow')
parser.add_argument('replay_file')
parser.add_argument('output')
args = parser.parse_args()

replay = os.path.realpath(args.replay_file)
output = os.path.realpath(args.output)
os.chdir(os.path.realpath(sys.argv[0] + "/" + os.path.pardir + "/" + os.path.pardir + "/" + os.path.pardir))

if args.renderer not in renderers:
	print("Invalid renderer selected. Available renderers:")
	for key in renderers.keys():
		print(f" - {key}")
	exit(1)


def start_ffmpeg_process(out_filename, width, height):
	args = [
		'ffmpeg',
		'-f', 'rawvideo',
		'-pix_fmt', 'rgba',
		'-r', '60',
		'-s', '{}x{}'.format(width, height),
		'-i', 'pipe:',
		'-pix_fmt', 'yuv420p',
		out_filename,
		'-y'
	]
	return subprocess.Popen(args, stdin=subprocess.PIPE)


cls, rargs = renderers[args.renderer]
handler = BattleHandler(False, False)
handler.load_replay(replay)
renderer = cls(*rargs)
handler.state.logger = lambda d: renderer.consume_event(d)
renderer.state = handler.state
renderer.sound_disabled = True
renderer.music_disabled = True
renderer.display_turn = False
index = 0
process = start_ffmpeg_process(output, *renderer.size)
handler.start()
while not handler.finished:
	handler.tick()
	f = True
	while f:
		print(f"\nTurn {renderer.turn} - Frame {index}\033[A\033[A")
		index += 1
		renderer.update()
		process.stdin.write(renderer.render_pic())
		f = not renderer.animation_ended

process.stdin.close()
process.wait()