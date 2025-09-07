from pyboy import PyBoy
from .YellowEmulator import PkmnYellowEmulator


ROM_PATH='pokeyellow.gbc'


class PyBoyEmulator(PkmnYellowEmulator):
	def __init__(self, has_interface=True, sound_volume=25, save_frames=False, debug=False):
		super().__init__()
		self.has_interface = has_interface
		self.save_frames = save_frames
		self.emulator = PyBoy(ROM_PATH, sound_volume=sound_volume, window='SDL2' if has_interface else 'null', debug=debug)
		self.emulator.hook_register(0x00, 0x1723, PkmnYellowEmulator.call_text_hook, self) # 0x3C36


	@property
	def register_hl(self):
		return self.emulator.register_file.HL


	@property
	def register_de(self):
		return (self.emulator.register_file.D << 8) | self.emulator.register_file.E


	def read(self, address, bank=None):
		if bank is not None:
			return self.emulator.memory[bank, address]
		return self.emulator.memory[address]


	def read_range(self, address_l, address_h, bank=None):
		if bank is not None:
			return self.emulator.memory[bank, address_l:address_h]
		return self.emulator.memory[address_l:address_h]


	def write(self, address, value, bank=None):
		if bank is not None:
			self.emulator.memory[bank, address] = value
		else:
			self.emulator.memory[address] = value


	def press_button(self, button, duration=1):
		self.emulator.button(button, duration)


	def tick(self, count=1):
		step = 1 if self.has_interface else 30
		for i in range(0, count, step):
			if not self.emulator.tick(step):
				raise InterruptedError()
			if self.save_frames:
				self.last_frames.append(self.emulator.screen.ndarray[:, :, :3].copy())


	def load_state(self, fd):
		self.emulator.load_state(fd)


	def stop(self):
		self.emulator.stop(False)
