use hidapi::{HidApi, HidDevice};
use powerslaves::*;

const DUMMY_CMD: TypicalCommand =
    TypicalCommand::new_ntr([0x9F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00], 0x2000);

const HEADER_CMD: TypicalCommand =
    TypicalCommand::new_ntr([0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00], 0x201);

fn try_empty_buffer(device: &HidDevice) {
    let mut data = [0; 0x40];
    while let Ok(sz) = device.read_timeout(&mut data, 10) {
        if sz == 0 {
            break;
        }
        println!("Got {sz} bytes of unknown data: {:?}", &data[..sz]);
    }
}

fn main() {
    let hidapi = HidApi::new().unwrap();

    let powersaves = Powersaves::open(&hidapi).unwrap();
    let test_buf = powersaves.switch_mode(Mode::Rom).unwrap();
    println!("Test Response: {test_buf:x?}");

    try_empty_buffer(&powersaves.device);
    {
        let mut dummy = vec![0x0; 0x2000];
        powersaves
            .send_receive(&DUMMY_CMD, &mut dummy)
            .unwrap()
            .unwrap();
        //std::fs::write("dummy.bin", &dummy).unwrap();
        try_empty_buffer(&powersaves.device);
    }

    let mut header = vec![0x0; 0x201];
    powersaves
        .send_receive(&HEADER_CMD, &mut header)
        .unwrap()
        .unwrap();
    try_empty_buffer(&powersaves.device);
    powersaves
        .send_receive(&HEADER_CMD, &mut header)
        .unwrap()
        .unwrap();
    try_empty_buffer(&powersaves.device);

    std::fs::write("header.bin", &header).unwrap();
}
