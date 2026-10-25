# CW Trainer

A Morse code listening station for a Scout JOTA/JOTI activity. A Scout
presses a button, hears a short word in Morse on a buzzer, and decodes it
letter by letter with a Morse chart. A Leader runs the station from a phone.

Letters are sent at a normal speed so each one sounds like a distinct shape,
but the gaps between letters are long (2.5 s by default), giving a Scout time
to find each letter on the chart. Younger Scouts may find it easier to write
down the dits and dahs first, then look up each letter afterwards.

## Parts

- ESP32-WROOM-32 DevKit (USB-C, CH340C)
- A passive piezo transducer: a bare piezo element with no built-in
  oscillator or driver, resonant at around 4 kHz. For example, the Jaycar
  AB3442 4 kHz piezo audio transducer.
- 2 jumper wires
- USB power bank and cable

An **active** buzzer (one with a built-in oscillator that beeps on steady DC)
also works, with a small code change. In `src/piezo.cc`, switch lead A with
`pinMode`/`digitalWrite` instead of the LEDC tone (HIGH in `On`, LOW in
`Off`), and connect lead B to GND. The buzzer then sounds at its own fixed
pitch, so the Pitch and Loud/Quiet settings have no effect. Active buzzers
have a polarity (+ goes to GPIO25). Many are rated for 5 V and are quieter
at 3.3 V. If yours draws more than about 20 mA, drive it through a transistor.

## Wiring

```
 ESP32 DevKit                 Piezo
 ┌──────────┐
 │   GPIO25 ├──────────────── lead A
 │   GPIO26 ├──────────────── lead B
 │          │
 │  [BOOT]  │  ← "play" button (built in)
 └──────────┘
```

- The passive piezo has no polarity, so either lead can go to either pin.
- The firmware drives the two pins in opposite directions. That doubles the
  voltage across the piezo and makes it noticeably louder.
- **Optional:** put a 100 Ω resistor in series with one lead. It protects
  the pins if the piezo is knocked hard (piezos generate voltage spikes when
  struck). It barely changes the volume.
- **Fallback:** if you only have one spare pin, connect lead B to GND. The
  station still works, but always at the quieter level.

## Development setup

You need [PlatformIO Core](https://docs.platformio.org/en/latest/core/installation/index.html)
to build and flash the firmware. The tests that run on the PC also need a
host C++ compiler (e.g. `g++`), and the style check needs `clang-format` and
`cpplint`:

```bash
pip install platformio cpplint
```

Commands, run from this folder:

| What | Command |
| --- | --- |
| Logic tests on the PC (no board needed) | `pio test -e native` |
| On-board tests (board plugged in) | `pio test -e esp32dev` |
| Build | `pio run` |
| Flash the board | `pio run -t upload` |
| Serial monitor | `pio device monitor` |
| Style check (Google C++) | see below |

```bash
clang-format --dry-run --Werror $(find src lib test -name "*.cc" -o -name "*.h") && cpplint --recursive --quiet src lib test
```

**The on-board tests erase the saved settings and messages.** Run them
before you set the station up for the day, or re-enter your messages
afterwards.

On Linux, your user must be in the `dialout` group to use the serial port
(`sudo usermod -aG dialout $USER`, then log out and back in).

## Running the station

### Connect

1. Power the board from the power bank.
2. On your phone, join the Wi-Fi network **CW-Trainer**. The password is
   **cwtrainer**. You can change both in `src/web_ui.cc` before flashing.
3. The control page should open by itself. If it doesn't, browse to
   **http://192.168.4.1**.
4. On Android, if a message says the network has no internet, choose
   **Stay connected**. Otherwise the phone may switch back to mobile data.

### A Scout's turn

1. The Scout presses the **BOOT** button on the board.
2. The current word plays once, slowly, in Morse.
3. The Scout find each letter on the chart and writes down, or writes
   down the dits and dahs to look up afterwards.
4. They can press BOOT again to hear the same word as many times as they
   like. The button never changes the word.
5. When they're ready, the Leader taps **Reveal answer** to check.
6. Then the Leader taps **Next message** or **Random message** for the next
   Scout. This hides the answer again.

### What the Leader can do

- **Play** and **Stop** the current word.
- **Reveal** or **hide** the answer. Until then the page shows one dot per
  letter, so you can see how long the word is without spoiling it.
- Turn on **coach mode** to see each letter appear as it plays. This helps
  you coach a Scout who is stuck.
- Change the **settings**, then tap **Save settings**:
  - **Letter speed:** 15 WPM keeps the letters crisp.
  - **Gap between letters:** make this longer for younger Scouts.
  - **Pitch:** 800 Hz is gentle. 4000 Hz is loudest but shrill.
  - **Loud/Quiet** volume.
  - **Plays per press:** 1 to 3.
- **Pick a message** by tapping it in the message list. This hides the
  answer again, just like Next and Random.
- **Add or delete messages.** Each message can be up to 20 letters, numbers
  and spaces. Anything else is removed, and the page tells you when that
  happens.

Settings and messages are saved on the board and survive being unplugged.

### Suggested starting settings

| Group | Letter gap | Pitch | Messages |
| --- | --- | --- | --- |
| Younger Scouts | 3.5–4 s | 800 Hz, Loud | HI, SOS, CAT, DOG |
| Older Scouts | 2–2.5 s | 800 Hz, Loud | RADIO, SCOUT, JOTA, HELLO |
| Noisy hall | as above | 2000–4000 Hz | |

## Factory reset

To erase all saved settings and messages and put the defaults back:

```bash
pio run -t erase
pio run -t upload
```

## Troubleshooting

| Problem | Try |
| --- | --- |
| No sound | Check the piezo leads are on GPIO25 and GPIO26. Check that the on-board LED flashes when it plays. If the LED flashes but there's no sound, the wiring is the problem. |
| Too quiet | Set volume to **Loud** and raise the pitch towards 2000–4000 Hz. The piezo is loudest near 4 kHz. |
| Too loud or shrill | Lower the pitch to around 800 Hz, or set volume to **Quiet**. |
| Page won't load | Make sure the phone is on **CW-Trainer** Wi-Fi, then browse to `http://192.168.4.1`. Turn off mobile data if the phone keeps leaving the network. |
| BOOT does nothing | Something may already be playing. Wait for it to finish, or tap **Stop**. |

## Code layout

| Path | Contents |
| --- | --- |
| `lib/morse/` | Morse table and timing sequencer (no Arduino code) |
| `lib/station/` | Settings, the message list, and JSON for the web API (no Arduino code) |
| `src/` | Hardware and Wi-Fi: `Piezo`, `Player`, `Button`, `WebUi`, flash storage, and the embedded web page |
| `test/` | Unity tests: `native` runs on the PC, and `test_storage` runs on the board |

The code follows the Google C++ Style Guide.

## Licence

This project is licensed under the GNU General Public License v3.0. See
[LICENSE](LICENSE).
