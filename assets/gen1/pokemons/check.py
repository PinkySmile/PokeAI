from PIL import Image


def check_line(p, y):
	return all(map(lambda f: f == 255, (p[x, y] for x in range(32))))


def check(path):
	m = Image.open(path).convert('L')
	if m.size[0] != 32 or m.size[1] != 32:
		return 0
	p = m.load()
	i = 0
	y = 28
	if check_line(p, y):
		y -= 1
		while check_line(p, y):
			i -= 1
			y -= 1
		return i
	y += 1
	while y != 32 and not check_line(p, y):
		i += 1
		y += 1
	return i

if __name__ == '__main__':
	import os
	import sys

	if len(sys.argv) != 1:
		for p in sys.argv[1:]:
			print(p, check(p + "/back.png"))
		exit(0)
	for p in os.listdir():
		if not os.path.isdir(p):
			continue
		v = check(p + "/back.png")
		if v != 0:
			print(p, v)
