import serial
import cbor2

def read_cbor_stream(port='/dev/ttyACM0', baudrate=115200):
    try:
        print(f"Trying to open {port}...")
        with serial.Serial(port, baudrate, timeout=1) as ser:
            print(f"✅ Connected to {port} at {baudrate} baud")

            buffer = b''

            while True:
                chunk = ser.read(64)
                # print("🔹 Got chunk:", chunk.hex())

                if chunk:
                    buffer += chunk

                    while b"\r\n" in buffer:
                        message, buffer = buffer.split(b"\r\n", 1)

                        if not message:
                            continue

                        try:
                            obj = cbor2.loads(message)
                            print("✅ Decoded CBOR:", obj)
                        except cbor2.CBORDecodeError as e:
                            print(f"❌ CBOR decode error: {e} - Raw: {message.hex()}")

    except serial.SerialException as e:
        print(f"Serial error: {e}")
    except KeyboardInterrupt:
        print("\n⛔ Stopped by user.")
    except Exception as e:
        print(f"❌ Unexpected error: {e}")

if __name__ == '__main__':
    read_cbor_stream()
