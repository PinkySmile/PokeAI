import os
import traceback
from typing import Callable

from pyboy import PyBoy
from .YellowEmulator import PkmnYellowEmulator, GBAddress, ROM_PATH, SYM_PATH


class PyBoyEmulator(PkmnYellowEmulator):
	def __init__(self, has_interface=True, sound_volume=25, save_frames=False, debug=False, rom=None):
		self.has_interface = has_interface
		self.save_frames = save_frames
		self.active_hooks = {}
		self.hook_callbacks = {}
		if rom is None:
			rom = ROM_PATH
			sym = SYM_PATH
		else:
			sym = os.path.abspath(os.path.join(rom, os.path.pardir, ".".join(os.path.basename(rom).split(".")[:-1]) + ".sym"))
		if sym and not os.path.isfile(sym):
			sym = None
		self.has_symbols = sym is not None
		self.emulator = PyBoy(rom, symbols=sym, sound_volume=sound_volume, window='SDL2' if has_interface else 'null', debug=debug)
		super().__init__()


	def symbol(self, name: str):
		if not self.has_symbols:
			return super().symbol(name)
		f = self.emulator.rom_symbols_inverse[name]
		return GBAddress(f[0], f[1])


	@property
	def register_a(self):
		return self.emulator.register_file.A
	@register_a.setter
	def register_a(self, value):
		self.emulator.register_file.A = value


	@property
	def register_b(self):
		return self.emulator.register_file.B
	@register_b.setter
	def register_b(self, value):
		self.emulator.register_file.B = value


	@property
	def register_c(self):
		return self.emulator.register_file.C
	@register_c.setter
	def register_c(self, value):
		self.emulator.register_file.C = value


	@property
	def register_d(self):
		return self.emulator.register_file.D
	@register_d.setter
	def register_d(self, value):
		self.emulator.register_file.D = value


	@property
	def register_e(self):
		return self.emulator.register_file.E
	@register_e.setter
	def register_e(self, value):
		self.emulator.register_file.E = value


	@property
	def register_h(self):
		return self.emulator.register_file.HL >> 8
	@register_h.setter
	def register_h(self, value):
		self.emulator.register_file.HL = ((value & 0xFF) << 8) | self.register_l


	@property
	def register_l(self):
		return self.emulator.register_file.HL & 0xFF
	@register_l.setter
	def register_l(self, value):
		self.emulator.register_file.HL = (self.register_h << 8) | (value & 0xFF)


	@property
	def register_bc(self):
		return (self.emulator.register_file.B << 8) | self.emulator.register_file.C
	@register_bc.setter
	def register_bc(self, value):
		self.emulator.register_file.B = (value >> 8) & 0xFF
		self.emulator.register_file.C = value & 0xFF


	@property
	def register_de(self):
		return (self.emulator.register_file.D << 8) | self.emulator.register_file.E
	@register_de.setter
	def register_de(self, value):
		self.emulator.register_file.D = (value >> 8) & 0xFF
		self.emulator.register_file.E = value & 0xFF


	@property
	def register_hl(self):
		return self.emulator.register_file.HL
	@register_hl.setter
	def register_hl(self, value):
		self.emulator.register_file.HL = value


	@property
	def register_sp(self):
		return self.emulator.register_file.SP
	@register_sp.setter
	def register_sp(self, value):
		self.emulator.register_file.SP = value


	def read(self, address):
		if address.bank is not None and address.address < 0xFF00:
			return self.emulator.memory[address.bank, address.address]
		return self.emulator.memory[address.address]


	def write(self, address, value):
		if address.bank is not None and address.address < 0xFF00:
			self.emulator.memory[address.bank, address.address] = value
		else:
			self.emulator.memory[address.address] = value


	def read_range(self, address_l, address_h):
		if address_l.bank == address_h.bank:
			assert address_l.address < address_h.address
			if address_l.bank is not None:
				return self.emulator.memory[address_l.bank, address_l.address:address_h.address]
			return self.emulator.memory[address_l.address:address_h.address]
		elif address_h.bank < 8 and address_l.bank < 8 and 0xC000 <= address_l.address < 0xE000 and 0xD000 <= address_h.address < 0xE000 and address_l.bank < address_h.bank:
			result = self.emulator.memory[address_l.bank, address_l.address:(address_l.address | 0xFFF)] + [self.emulator.memory[address_l.address | 0xFFF]]
			for i in range(address_l.bank + 1, address_h.bank - 1):
				result += self.emulator.memory[i, 0xD000:0xDFFF] + [self.emulator.memory[0xDFFF]]
			result += self.emulator.memory[address_h.bank, (address_h.address & 0xF000):address_h.address]
			assert len(result) == address_h.address - address_l.address
			return result
		assert False


	def press_button(self, button, duration=1):
		self.emulator.button(button, duration)


	def tick(self, count=1):
		step = 1 if self.has_interface and not self.fast_forward else 30
		for i in range(0, count, step):
			if not self.emulator.tick(step):
				raise InterruptedError()
			if self.save_frames:
				self.last_frames.append(self.emulator.screen.ndarray[:, :, :3].copy())


	def jump(self, address: GBAddress):
		if address.bank is None or address.bank == 0x00:
			self.emulator.register_file.PC = address.address
			return
		self.emulator.register_file.HL = address.address
		self.emulator.register_file.B = address.bank
		symbol = self.symbol("Bankswitch")
		assert symbol.bank == 0
		self.emulator.register_file.PC = symbol.address


	def hook(self, address: GBAddress, callback: Callable, userdata=None):
		# For whatever reason, PyBoy doesn't want to unregister the callbacks because it cannot find them
		# so we make a wrapper that doesn't execute when we deregister it
		def a(b):
			if self.active_hooks[callback]:
				callback(b)
		result = self.hook_callbacks.get((address.bank << 16) | address.address, [])
		self.hook_callbacks[(address.bank << 16) | address.address] = result
		result.append(callback)
		self.active_hooks[callback] = True
		try:
			self.emulator.hook_register(address.bank, address.address, a, userdata)
		except:
			# But here apparently it does work!?
			self.emulator.hook_deregister(address.bank, address.address)
			self.emulator.hook_register(address.bank, address.address, a, userdata)


	def unhook(self, address: GBAddress):
		callbacks = self.hook_callbacks.get((address.bank << 16) | address.address, [])
		del self.hook_callbacks[(address.bank << 16) | address.address]
		try:
			self.emulator.hook_deregister(address.bank, address.address)
			for c in callbacks:
				del self.active_hooks[c]
		except ValueError:
			for c in callbacks:
				self.active_hooks[c] = False


	def load_state(self, fd):
		self.emulator.load_state(fd)


	def stop(self):
		self.emulator.stop(False)
