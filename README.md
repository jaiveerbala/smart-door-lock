# Smart Door Lock

An RFID and NFC-based smart deadbolt lock built with an Arduino Nano. Unlock with an RFID fob or your phone.

## Overview

This is the second version of a door lock project I started in 6th grade. The original used an RFID sensor and servo mounted in a drilled-out outlet fuse box. The current version is a fully redesigned system with a custom compact enclosure, integrated power management, and phone unlock via NFC.

## Features

- Unlock with RFID fob or phone NFC tag
- Auto-locks after 1 minute
- Integrated LiPo battery with dedicated charging system — recharges weekly
- Custom compact enclosure with countersunk screws
- NFC sticker on the front for phone unlock

## Hardware

- Arduino Nano
- RC522 RFID module
- Standard servo motor
- LiPo battery + TP4056 charging module
- Custom 3D printed enclosure

## How It Works

The RC522 module continuously scans for RFID/NFC tags. When an authorized tag is detected, the Arduino signals the servo to rotate and actuate the deadbolt linkage. After 60 seconds the servo automatically returns to the locked position.

## Version History

**v1 (2022)** — RFID fob only, powered by wall adapter, mounted in a drilled outlet fuse box. Functional but not practical.

**v2 (current)** — Redesigned from scratch. Custom enclosure, battery powered, NFC phone unlock, auto-lock.
