#!/usr/bin/env python3
"""Generate the shutter sound shipped with imagen.

The sound is synthesised from scratch: a short burst of filtered noise
followed by a damped sine wave, which resembles the click of a camera
shutter. Run this script to regenerate assets/sounds/shutter.wav.

    python3 assets/tools/generate_shutter_sound.py
"""

from __future__ import annotations

import math
import random
import struct
import wave
from pathlib import Path

SAMPLE_RATE = 44100
DURATION = 0.22
OUTPUT = Path(__file__).resolve().parents[1] / "sounds" / "shutter.wav"


def one_pole_lowpass(samples: list[float], cutoff: float) -> list[float]:
    """Simple one pole low pass filter."""
    alpha = 1.0 - math.exp(-2.0 * math.pi * cutoff / SAMPLE_RATE)
    out: list[float] = []
    state = 0.0
    for sample in samples:
        state += alpha * (sample - state)
        out.append(state)
    return out


def build() -> list[float]:
    random.seed(20261105)
    count = int(SAMPLE_RATE * DURATION)

    # First stage: the mechanical click of the blades opening.
    noise = [random.uniform(-1.0, 1.0) for _ in range(count)]
    noise = one_pole_lowpass(noise, 3200.0)

    # Second stage: the body resonance that follows the click.
    resonance = []
    for index in range(count):
        t = index / SAMPLE_RATE
        envelope = math.exp(-t * 26.0)
        resonance.append(
            envelope
            * (
                0.55 * math.sin(2.0 * math.pi * 1850.0 * t)
                + 0.30 * math.sin(2.0 * math.pi * 3100.0 * t)
                + 0.15 * math.sin(2.0 * math.pi * 4700.0 * t)
            )
        )

    samples = []
    for index in range(count):
        t = index / SAMPLE_RATE
        click_envelope = math.exp(-t * 55.0)
        value = 0.55 * noise[index] * click_envelope + 0.45 * resonance[index]
        # Fade out the tail so the file ends cleanly.
        value *= min(1.0, max(0.0, (DURATION - t) / 0.02))
        samples.append(max(-1.0, min(1.0, value)))
    return samples


def main() -> None:
    samples = build()
    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    with wave.open(str(OUTPUT), "wb") as handle:
        handle.setnchannels(1)
        handle.setsampwidth(2)
        handle.setframerate(SAMPLE_RATE)
        frames = b"".join(struct.pack("<h", int(sample * 32000)) for sample in samples)
        handle.writeframes(frames)
    print(f"wrote {OUTPUT} ({OUTPUT.stat().st_size} bytes)")


if __name__ == "__main__":
    main()
