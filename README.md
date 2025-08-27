# Autonommous Radio Computer (or Anarchist's Radio Computer) Operating System.

Next-gen open-source device with capabilities of the real computer or smartphone but also with access to the censorship-free decentralised web (ARC-Network).
ARC-NET will support most popular MESH radio protocols and also will implement it's own protocol on top of them that will allow access to the Internet (slow) via MESH networks. Future plans also include adding support for decentralised web-resources inside the network, gateways to the smartphones to allow usage of proprietary apps, SDK for convenient apps development and more!

# --- OVERALL STRUCTURE ---

<img width="1764" height="968" alt="project" src="https://github.com/user-attachments/assets/9f15cfcb-2bd2-44ac-b891-f07365af73bd" />

General project structure.

<img width="884" height="648" alt="hierarchy" src="https://github.com/user-attachments/assets/d71745d3-c700-46a4-9ada-cc427e89cbf2" />

OS abstraction layers

<img width="4104" height="1604" alt="repo structure" src="https://github.com/user-attachments/assets/5ce6eb93-2559-48f2-adc4-b932cb97707f" />

Repository structure.

<img width="3004" height="804" alt="todo" src="https://github.com/user-attachments/assets/19c4a769-3486-4907-b400-7b62041e356b" />

**TODO for v0.1**
**TODO for Driver devs:**
- Character drivers for SPI, I2C, GPIO, UART, USB-CDC
- Netowrk drivers for WiFi, BLE, LoRa.
- System library for RSA/AES cryptography and for MESH

# --- FOR DRIVER DEVELOPERS ---

<img width="900" height="331" alt="drivers" src="https://github.com/user-attachments/assets/fe773107-4f13-440d-b050-71c743ec31ef" />
Description of the driver interfaces. Architecture is inspired by Linux kernel but simplified significantly to reduce OS footprint and development complexity. There are 3 common types of drivers: Character (buses etc.), Block (memory), Network (WiFi, LoRa, Bluetooth), and 1 unique type that will exist specifically inside ARC ecosystem: Mount. Mount driver is basically high-level imlementation of the filesystem, based on VFS HALs and drivers provided by Espressif.

Main goal of the driver developer is to create implementations for the necessary functions and initialise structs(DevFileEntry, <type>DriverOps, <iface>_ctx_t).

<img width="392" height="324" alt="structs" src="https://github.com/user-attachments/assets/4850c33f-89b4-4539-a298-1d3307ab1e0d" />

Relations of the structs (Diagram may contain mistakes, consult with interfaces diagram when implementing the driver)

- **DevFileEntry:** Contains file path, type of thedriver and pointers to other structs
- **(example)CharDriverOps:** Contains pointers to implementations of the driver's methods. For BlockDriverOps and NetworkDriverOps principles are the same but with different commands set.
- **(example)spi_ctx_t:** Structure with the parameters of the driver (gpio, clock rate etc.). This structure DOES NOT define the parameters themself, but provides the way for kernel developer to do so.

<img width="647" height="251" alt="drivers_example" src="https://github.com/user-attachments/assets/3ea42769-8855-4603-8937-40f641e36b38" />

General example of communication between App and spi driver. More detailed view will be discussed in KERNEL DEV's documentation.

# --- FOR KERNEL DEVELOPERS ---

In progress

# --- FOR APP DEVELOPERS ---

In progress
