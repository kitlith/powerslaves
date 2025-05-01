use hidapi::{HidApi, HidDevice, HidResult};
use zerocopy::{FromZeros, Immutable, IntoBytes, Unaligned, byteorder::little_endian::U16};

#[derive(IntoBytes, Immutable, Unaligned, Default)]
#[repr(u8)]
pub enum CommandType {
    Test = 0x02,
    // 0x08 and 0x09 appear to disconnect USB and infloop
    SwitchMode = 0x10,
    RomMode = 0x11,
    SpiMode = 0x12,
    #[default]
    Ntr = 0x13, // 8 byte command
    Ctr = 0x14, // 16 byte command
    Spi = 0x15,
    // 0x16, // "weird reset for SPI and locks up powerslaves"
    // 0x20,
    // 0x30
}
// if first 4 bytes are [0x99, 0x44, 0x46, 0x55], it seems to perform a firmware update
#[derive(IntoBytes, FromZeros, Immutable, Unaligned)]
#[repr(C)]
pub struct RawCommand {
    // always 0
    report_id: u8,
    pub cmdbuf: [u8; 64],
}

#[derive(IntoBytes, Immutable, Unaligned, Default)]
#[repr(C)]
pub struct TypicalCommandHeader {
    pub command_type: CommandType,
    pub command_len: U16,
    pub response_len: U16,
}

#[derive(IntoBytes, Immutable, Unaligned)]
#[repr(C)]
pub struct TypicalCommand {
    // always 0
    report_id: u8,
    pub header: TypicalCommandHeader,
    pub data: [u8; Self::DATA_SIZE],
}

impl TypicalCommand {
    const DATA_SIZE: usize = Powersaves::REPORT_SIZE - size_of::<TypicalCommandHeader>();

    pub const fn new_simple(command_type: CommandType, response_len: u16) -> Self {
        Self {
            report_id: 0,
            header: TypicalCommandHeader {
                command_type,
                command_len: U16::new(0),
                response_len: U16::new(response_len),
            },
            data: [0; Self::DATA_SIZE],
        }
    }

    pub const fn new_ntr<const N: usize>(cmd: [u8; N], response_len: u16) -> Self {
        let mut data = [0; Self::DATA_SIZE];
        // const context :/
        let mut idx = 0usize;
        while idx < N {
            data[idx] = cmd[idx];
            idx += 1;
        }
        Self {
            report_id: 0,
            header: TypicalCommandHeader {
                command_type: CommandType::Ntr,
                command_len: U16::new(N as u16),
                response_len: U16::new(response_len),
            },
            data,
        }
    }
}

pub struct Powersaves {
    pub device: HidDevice,
}

impl Powersaves {
    const VID: u16 = 0x1C1A;
    const PID: u16 = 0x03D5;

    const REPORT_SIZE: usize = 64;
    // previously: powerslaves_select
    pub fn open(hidapi: &HidApi) -> HidResult<Self> {
        let device = hidapi.open(Self::VID, Self::PID)?;
        Ok(Self { device })
    }

    pub fn open_serial(hidapi: &HidApi, serial: &str) -> HidResult<Self> {
        let device = hidapi.open_serial(Self::VID, Self::PID, serial)?;
        Ok(Self { device })
    }

    pub fn send_raw(&self, cmd: &RawCommand) -> HidResult<()> {
        let count = self.device.write(cmd.as_bytes())?;
        assert_eq!(count, Self::REPORT_SIZE + 1);
        Ok(())
    }

    pub fn send(&self, cmd: &TypicalCommand) -> HidResult<()> {
        let count = self.device.write(cmd.as_bytes())?;
        assert_eq!(count, Self::REPORT_SIZE + 1);
        Ok(())
    }

    pub fn read_all(&self, buf: &mut [u8]) -> HidResult<()> {
        let mut bytes_read = 0usize;
        while bytes_read < buf.len() {
            // device seems to return in 0x40 byte chunks.
            // TODO: what about smaller sizes?
            bytes_read += self.device.read(&mut buf[bytes_read..])?;
            //println!("... {bytes_read}");
        }
        Ok(())
    }

    pub fn send_receive(
        &self,
        cmd: &TypicalCommand,
        resp: &mut [u8],
    ) -> Result<HidResult<()>, &'static str> {
        if resp.len() != cmd.header.response_len.get().into() {
            return Err("response buffer length != size requested");
        }
        Ok(self.internal_send_receive(cmd, resp))
    }

    fn internal_send_receive(&self, cmd: &TypicalCommand, resp: &mut [u8]) -> HidResult<()> {
        self.send(cmd)?;
        self.read_all(resp)
    }

    pub const SWITCH_MODE_CMD: TypicalCommand =
        TypicalCommand::new_simple(CommandType::SwitchMode, 0x0);
    pub const MODE_ROM_CMD: TypicalCommand = TypicalCommand::new_simple(CommandType::RomMode, 0x0);
    pub const MODE_SPI_CMD: TypicalCommand = TypicalCommand::new_simple(CommandType::SpiMode, 0x0);
    pub const TEST_CMD: TypicalCommand = TypicalCommand::new_simple(CommandType::Test, 0x40);

    pub fn switch_mode(&self, mode: Mode) -> HidResult<[u8; 64]> {
        self.send(&Self::SWITCH_MODE_CMD)?;
        let mode_cmd = match mode {
            Mode::Rom => &Self::MODE_ROM_CMD,
            Mode::Spi => &Self::MODE_SPI_CMD,
        };
        self.send(mode_cmd)?;
        let mut test_buf = [0; 64];
        self.internal_send_receive(&Self::TEST_CMD, &mut test_buf)?;
        Ok(test_buf)
    }
}

pub enum Mode {
    Rom,
    Spi,
}
