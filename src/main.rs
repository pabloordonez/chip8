extern crate sdl2; 

use sdl2::pixels::Color;
use sdl2::event::Event;
use sdl2::keyboard::Keycode;
use std::time::Duration;

struct Chip8 {
    memory: [u8; 4096],
    program_counter: u16,
    value_registers: [u8; 16],
    index_register: u16,
    stack: [u16; 16],
    stack_pointer: usize,
    sound_timer: u8,
    delay_timer: u8,
    gpu: Gpu,
    keyboard: Keyboard,
}

impl Chip8 {
    pub fn new() -> Self {
        Chip8 {
            memory: [0; 4096],
            program_counter: 0,
            value_registers: [0; 16],
            index_register: 0,
            stack: [0; 16],
            stack_pointer: 0,
            sound_timer: 0,
            delay_timer: 0,
            gpu: Gpu::new(),
            keyboard: Keyboard::new(),
        }
    }
    pub fn execute_op(&mut self, op_code: u16) {
        let op1 = ((op_code & 0xF000) >> 12) as u8;
        let op2 = ((op_code & 0x0F00) >> 8) as u8;
        let op3 = ((op_code & 0x00F0) >> 4) as u8;
        let op4 = (op_code & 0x000F) as u8;
        let nnn = op_code & 0x0FFF;
        let kk = (op_code & 0x00FF) as u8;

        match (op1, op2, op3, op4) {
            (0x00, 0x00, 0x0E, 0x00) => self.op_cls(),
            (0x00, 0x00, 0x0E, 0x0E) => self.op_ret(),
            (0x00, _, _, _) => self.op_sys_nnn(nnn),
            (0x01, _, _, _) => self.op_jp_nnn(nnn),
            (0x02, _, _, _) => self.op_call_nnn(nnn),
            (0x03, _, _, _) => self.op_se_vx_kk(op2, kk),
            (0x04, _, _, _) => self.op_sne_vx_kk(op2, kk),
            (0x05, _, _, 0x00) => self.op_se_vx_vy(op2, op3),
            (0x06, _, _, _) => self.op_ld_vx_kk(op2, kk),
            (0x07, _, _, _) => self.op_add_vx_kk(op2, kk),

            (0x08, _, _, 0x00) => self.op_ld_vx_vy(op2, op3),
            (0x08, _, _, 0x01) => self.op_or_vx_vy(op2, op3),
            (0x08, _, _, 0x02) => self.op_and_vx_vy(op2, op3),
            (0x08, _, _, 0x03) => self.op_xor_vx_vy(op2, op3),
            (0x08, _, _, 0x04) => self.op_add_vx_vy(op2, op3),
            (0x08, _, _, 0x05) => self.op_sub_vx_vy(op2, op3),
            (0x08, _, _, 0x06) => self.op_shr_vx(op2),
            (0x08, _, _, 0x07) => self.op_subn_vx_vy(op2, op3),
            (0x08, _, _, 0x0E) => self.op_shl_vx(op2),
            (0x09, _, _, 0x00) => self.op_sne_vx_vy(op2, op3),

            (0x0A, _, _, _) => self.op_ld_i_nnn(nnn),
            (0x0B, _, _, _) => self.op_jp_v0_nnn(nnn),
            (0x0C, _, _, _) => self.op_rnd_vx_kk(op2, kk),
            (0x0D, _, _, _) => self.op_drw_vx_vy_n(op2, op3, op4),

            (0x0E, _, 0x09, 0x0E) => self.op_skp_x(op2),
            (0x0E, _, 0x0A, 0x01) => self.op_skpn_x(op2),
            (0x0F, _, 0x00, 0x07) => self.op_ld_vx_dt(op2),
            (0x0F, _, 0x00, 0x0A) => self.op_ld_vx_k(op2),
            (0x0F, _, 0x01, 0x05) => self.op_ld_dt_vx(op2),
            (0x0F, _, 0x01, 0x08) => self.op_ld_st_vx(op2),
            (0x0F, _, 0x01, 0x0E) => self.op_add_i_vx(op2),
            (0x0F, _, 0x02, 0x09) => self.op_ld_f_vx(op2),
            (0x0F, _, 0x03, 0x03) => self.op_ld_b_vx(op2),
            (0x0F, _, 0x05, 0x05) => self.op_ld_i_vx(op2),
            (0x0F, _, 0x06, 0x05) => self.op_ld_vx_i(op2),
            (_, _, _, _) => (),
        }
    }

    /* TODO: maybe use macros? */
    /* OPCODE  CNNN */
    #[inline(always)]
    fn op_sys_nnn(&mut self, nnn: u16) {
        self.program_counter = nnn
    }

    #[inline(always)]
    fn op_cls(&mut self) {
        self.gpu.clear_memory()
    }

    #[inline(always)]
    fn op_ret(&mut self) {
        self.program_counter = self.stack[self.stack_pointer];
        self.stack_pointer -= 1;
    }

    #[inline(always)]
    fn op_jp_nnn(&mut self, nnn: u16) {
        self.program_counter = nnn
    }

    #[inline(always)]
    fn op_jp_v0_nnn(&mut self, nnn: u16) {
        self.program_counter = nnn + self.value_registers[0] as u16
    }

    #[inline(always)]
    fn op_call_nnn(&mut self, nnn: u16) {
        self.stack_pointer += 1;
        self.stack[self.stack_pointer] = self.program_counter;
        self.program_counter = nnn;
    }

    #[inline(always)]
    fn op_se_vx_kk(&mut self, x: u8, kk: u8) {
        // we move two bytes ahead: each instruction is 2 bytes, so we are moving one instruction ahead.
        if self.value_registers[x as usize] == kk {
            self.program_counter += 2
        }
    }

    #[inline(always)]
    fn op_se_vx_vy(&mut self, x: u8, y: u8) {
        // we move two bytes ahead: each instruction is 2 bytes, so we are moving one instruction ahead.
        if self.value_registers[x as usize] == self.value_registers[y as usize] {
            self.program_counter += 2
        }
    }

    #[inline(always)]
    fn op_sne_vx_kk(&mut self, x: u8, kk: u8) {
        // we move two bytes ahead: each instruction is 2 bytes, so we are moving one instruction ahead.
        if self.value_registers[x as usize] != kk {
            self.program_counter += 2
        }
    }

    #[inline(always)]
    fn op_sne_vx_vy(&mut self, x: u8, y: u8) {
        // we move two bytes ahead: each instruction is 2 bytes, so we are moving one instruction ahead.
        if self.value_registers[x as usize] != self.value_registers[y as usize] {
            self.program_counter += 2
        }
    }

    #[inline(always)]
    fn op_skp_x(&mut self, x: u8) {
        // we move two bytes ahead: each instruction is 2 bytes, so we are moving one instruction ahead.
        if self
            .keyboard
            .is_key_pressed(self.value_registers[x as usize])
        {
            self.program_counter += 2
        }
    }

    #[inline(always)]
    fn op_skpn_x(&mut self, x: u8) {
        // we move two bytes ahead: each instruction is 2 bytes, so we are moving one instruction ahead.
        if !self
            .keyboard
            .is_key_pressed(self.value_registers[x as usize])
        {
            self.program_counter += 2
        }
    }

    #[inline(always)]
    fn op_ld_i_nnn(&mut self, nnn: u16) {
        self.index_register = nnn
    }

    #[inline(always)]
    fn op_ld_vx_kk(&mut self, x: u8, kk: u8) {
        self.value_registers[x as usize] = kk
    }

    #[inline(always)]
    fn op_ld_vx_vy(&mut self, x: u8, y: u8) {
        self.value_registers[x as usize] = self.value_registers[y as usize]
    }

    #[inline(always)]
    fn op_ld_vx_dt(&mut self, x: u8) {
        self.value_registers[x as usize] = self.delay_timer
    }

    #[inline(always)]
    fn op_ld_dt_vx(&mut self, x: u8) {
        self.delay_timer = self.value_registers[x as usize]
    }

    #[inline(always)]
    fn op_ld_st_vx(&mut self, x: u8) {
        self.sound_timer = self.value_registers[x as usize]
    }

    #[inline(always)]
    fn op_ld_f_vx(&mut self, x: u8) {
        self.index_register = self.value_registers[x as usize] as u16 * 5u16
    }

    fn op_ld_b_vx(&mut self, x: u8) {
        let i = self.index_register as usize;
        let a = x / 100;
        let x = x - a * 100;
        let b = x / 10;
        let x = x - b * 10;

        self.memory[i + 0] = a;
        self.memory[i + 1] = b;
        self.memory[i + 2] = x;
    }

    fn op_ld_i_vx(&mut self, x: u8) {
        let i = self.index_register as usize;
        let x = x as usize;
        self.memory[i..i + x].copy_from_slice(&self.value_registers[0..x]);
    }

    fn op_ld_vx_i(&mut self, x: u8) {
        let i = self.index_register as usize;
        let x = x as usize;
        self.value_registers[0..x].copy_from_slice(&self.memory[i..i + x]);
    }

    #[inline(always)]
    fn op_ld_vx_k(&mut self, x: u8) {
        // we move two bytes ahead: each instruction is 2 bytes, so we are moving one instruction ahead.
        if !self.keyboard.is_any_key_pressed() {
            self.program_counter -= 2;
            return;
        }
        let x = x as usize;
        let key_index = self.keyboard.get_key_pressed_index();

        match key_index {
            Some(index) => self.value_registers[x] = index as u8,
            None => (),
        }
    }

    #[inline(always)]
    fn op_add_vx_kk(&mut self, x: u8, kk: u8) {
        self.value_registers[x as usize] += kk
    }

    #[inline(always)]
    fn op_add_vx_vy(&mut self, x: u8, y: u8) {
        let (value, overflow) =
            self.value_registers[x as usize].overflowing_add(self.value_registers[y as usize]);
        self.value_registers[x as usize] = value;
        self.value_registers[0x0F] = overflow as u8;
    }

    #[inline(always)]
    fn op_add_i_vx(&mut self, x: u8) {
        self.index_register += self.value_registers[x as usize] as u16
    }

    #[inline(always)]
    fn op_or_vx_vy(&mut self, x: u8, y: u8) {
        self.value_registers[x as usize] |= self.value_registers[y as usize]
    }

    #[inline(always)]
    fn op_and_vx_vy(&mut self, x: u8, y: u8) {
        self.value_registers[x as usize] &= self.value_registers[y as usize]
    }

    #[inline(always)]
    fn op_xor_vx_vy(&mut self, x: u8, y: u8) {
        self.value_registers[x as usize] ^= self.value_registers[y as usize]
    }

    #[inline(always)]
    fn op_sub_vx_vy(&mut self, x: u8, y: u8) {
        let x = x as usize;
        let y = y as usize;
        let vx = self.value_registers[x];
        let vy = self.value_registers[y];

        self.value_registers[0x0F] = (vx > vy) as u8;
        self.value_registers[x] = vx - vy;
    }

    #[inline(always)]
    fn op_subn_vx_vy(&mut self, x: u8, y: u8) {
        let x = x as usize;
        let y = y as usize;
        let vx = self.value_registers[x];
        let vy = self.value_registers[y];

        self.value_registers[0x0F] = (vy > vx) as u8;
        self.value_registers[x] = vy - vx;
    }

    #[inline(always)]
    fn op_shr_vx(&mut self, x: u8) {
        let x = x as usize;
        self.value_registers[0x0F] = self.value_registers[x] & 0b00000001;
        self.value_registers[x] >>= 1;
    }

    #[inline(always)]
    fn op_shl_vx(&mut self, x: u8) {
        let x = x as usize;
        self.value_registers[0x0F] = self.value_registers[x] & 0b10000000;
        self.value_registers[x] >>= 1;
    }

    #[inline(always)]
    fn op_rnd_vx_kk(&mut self, x: u8, kk: u8) {
        self.value_registers[x as usize] = rand::random::<u8>() & kk;
    }

    #[inline(always)]
    fn op_drw_vx_vy_n(&mut self, x: u8, y: u8, n: u8) {
        let i = self.index_register as usize;
        let n = n as usize;
        self.value_registers[0x0F] = self.gpu.draw(x, y, &self.memory[i..i + n]) as u8;
    }
}

struct Gpu {
    memory: [u8; 256],
}

impl Gpu {
    pub fn new() -> Self {
        Gpu { memory: [0; 256] }
    }

    pub fn clear_memory(&mut self) {
        unsafe { std::ptr::write_volatile(&mut self.memory, [0u8; 256]) }
    }

    pub fn draw(&mut self, x: u8, y: u8, memory: &[u8]) -> bool {
        true
    }
}

struct Keyboard {
    memory: u16,
}

impl Keyboard {
    pub fn new() -> Self {
        Keyboard { memory: 0u16 }
    }

    pub fn is_any_key_pressed(&self) -> bool {
        self.memory != 0
    }

    pub fn is_key_pressed(&self, key: u8) -> bool {
        return ((self.memory >> key) & 0x01) != 0;
    }

    pub fn get_key_pressed_index(&self) -> Option<u8> {
        let keys = self.memory;
        for i in 0..16 {
            if (keys >> i) & 0x0001 != 0 {
                return Some(i);
            }
        }
        return None;
    }
}

fn main() {
    let mut chip = Chip8::new();
    chip.execute_op(0x00000E00);

    let sdl_context = sdl2::init().unwrap();
    let video_subsystem = sdl_context.video().unwrap();
 
    let window = video_subsystem.window("rust-sdl2 demo", 800, 600)
        .position_centered()
        .build()
        .unwrap();
 
    let mut canvas = window.into_canvas().build().unwrap();
 
    canvas.set_draw_color(Color::RGB(0, 255, 255));
    canvas.clear();
    canvas.present();
    let mut event_pump = sdl_context.event_pump().unwrap();
    let mut i = 0;
    'running: loop {
        i = (i + 1) % 255;
        canvas.set_draw_color(Color::RGB(i, 64, 255 - i));
        canvas.clear();
        for event in event_pump.poll_iter() {
            match event {
                Event::Quit {..} |
                Event::KeyDown { keycode: Some(Keycode::Escape), .. } => {
                    break 'running
                },
                _ => {}
            }
        }
        // The rest of the game loop goes here...

        canvas.present();
        ::std::thread::sleep(Duration::new(0, 1_000_000_000u32 / 60));
    }
}

