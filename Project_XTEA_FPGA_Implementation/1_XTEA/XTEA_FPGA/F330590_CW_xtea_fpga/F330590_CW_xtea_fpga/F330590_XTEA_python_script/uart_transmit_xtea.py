import serial
import struct
import time

PORT = "COM15"
BAUD = 9600
INTER_TEST_DELAY = 1  # Delay BETWEEN tests in seconds - NOT included in timing

TEST_CASES = [
    {
        "name"   : "Key Pair 1",
        "key"    : [0xDEADBEEF, 0x01234567, 0x89ABCDEF, 0xDEADBEEF],
        "plain"  : [0xA5A5A5A5, 0x01234567, 0xFEDCBA98, 0x5A5A5A5A],
        "cipher" : [0x089975E9, 0x2555F334, 0xCE76E4F2, 0x4D932AB3],
    },
    {
        "name"   : "Key Pair 2",
        "key"    : [0x73467723, 0x46534858, 0x97346378, 0x24782378],
        "plain"  : [0xFEDCBAFE, 0xDCBAFEDC, 0xBAFEDCBA, 0xFEDCBAFE],
        "cipher" : [0x07F5110D, 0x638D6121, 0xF0FC4354, 0x0AB370B8],
    },
    {
        "name"   : "Key Pair 3",
        "key"    : [0xABCDEFAB, 0xCDEFABCD, 0xEFABCDEF, 0xABCDEFAB],
        "plain"  : [0x46893489, 0x23789423, 0x89646238, 0x12300325],
        "cipher" : [0xBC8BD8D9, 0x269AC661, 0x73B1FF23, 0x8096EC53],
    },
]

def words_to_bytes(words):
    return b''.join(struct.pack('>I', w & 0xFFFFFFFF) for w in words)

def bytes_to_words(data):
    return [struct.unpack('>I', data[i*4:(i+1)*4])[0] for i in range(4)]

def send_xtea(ser, encrypt, key, data):
    """
    Send one XTEA transaction and return (result_words, timing_dict).

    Timing is measured in nanoseconds using time.perf_counter() which is
    the highest resolution timer available in Python.

    The INTER_TEST_DELAY sleep happens AFTER these measurements so it
    does NOT affect the reported times.

    Breakdown:
      tx_ns    : time to hand 33 bytes to the OS serial driver
      rx_ns    : time waiting for all 16 result bytes to arrive
      total_ns : full round-trip (tx_ns + fpga compute + rx_ns)

    Note: tx_ns measures when Python hands data to the driver, not when
    the last bit leaves the UART hardware. The actual on-wire TX time is:
      33 bytes x 10 bits x (1/9600) = 34,375,000 ns = ~34.4 ms
    This is why rx_ns includes most of the TX propagation time as well.
    total_ns is the most meaningful single figure.
    """
    ctrl    = bytes([0x01 if encrypt else 0x00])
    payload = ctrl + words_to_bytes(key) + words_to_bytes(data)
    ser.reset_input_buffer()

    # --- TX: measure time to write payload to serial driver ---
    t_tx_start = time.perf_counter()
    ser.write(payload)
    ser.flush()
    t_tx_end = time.perf_counter()

    # --- RX: measure time to receive all 16 result bytes ---
    t_rx_start = time.perf_counter()
    response = b''
    deadline = time.time() + 5.0
    while len(response) < 16 and time.time() < deadline:
        chunk = ser.read(16 - len(response))
        if chunk:
            response += chunk
    t_rx_end = time.perf_counter()

    timing = {
        "tx_ns"    : (t_tx_end - t_tx_start) * 1_000_000_000,
        "rx_ns"    : (t_rx_end - t_rx_start) * 1_000_000_000,
        "total_ns" : (t_rx_end - t_tx_start) * 1_000_000_000,
    }

    return bytes_to_words(response), timing


def main():
    print(f"\nConnecting to {PORT} at {BAUD} baud")
    try:
        ser = serial.Serial(PORT, BAUD, timeout=2)
    except serial.SerialException as e:
        print(f"\nERROR: Cannot open {PORT}\n  {e}")
        return

    passed       = 0
    failed       = 0
    all_times_ns = []   # collects total_ns

    with ser:
        time.sleep(0.1)
        print("Connected.\n")

        for tc in TEST_CASES:
            for encrypt, data_in, expected in [
                (True,  tc["plain"],  tc["cipher"]),
                (False, tc["cipher"], tc["plain"]),
            ]:
                direction = "ENCRYPT" if encrypt else "DECRYPT"
                print(f"{'='*55}")
                print(f"  {direction}  -  {tc['name']}")
                print(f"{'='*55}")
                print(f"  Key     : " + " ".join(f"{w:08X}" for w in tc["key"]))
                print(f"  Input   : " + " ".join(f"{w:08X}" for w in data_in))

                try:
                    result, timing = send_xtea(ser, encrypt=encrypt,
                                               key=tc["key"], data=data_in)
                    ok = (result == expected)
                    print(f"  Result  : " + " ".join(f"{w:08X}" for w in result))
                    print(f"  Expected: " + " ".join(f"{w:08X}" for w in expected))

                    all_times_ns.append(timing['total_ns'])

                    print(f"\n  RESULT: {'PASS' if ok else 'FAIL'}")
                    if ok: passed += 1
                    else:  failed += 1

                except (struct.error, Exception) as e:
                    print(f"  RESULT: TIMEOUT / ERROR\n    {e}")
                    failed += 1

                print()

                # Delay between tests
                time.sleep(INTER_TEST_DELAY)

        total = passed + failed
        print(f"{'='*55}")
        print(f"  SUMMARY: {passed}/{total} tests passed")

        if all_times_ns:
            avg_ns = sum(all_times_ns) / len(all_times_ns)
            min_ns = min(all_times_ns)
            max_ns = max(all_times_ns)

            print(f"\n  Inference time ")
            print(f"    Min   : {min_ns:>15,.0f} ns")
            print(f"    Max   : {max_ns:>15,.0f} ns")
            print(f"    Avg   : {avg_ns:>15,.0f} ns")

        print(f"\n  {'ALL PASSED' if failed == 0 else str(failed) + ' FAILED'}")
        print(f"{'='*55}")


main()
