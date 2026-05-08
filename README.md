*This project has been created as part of the 42 curriculum by <iel-ghou>{ilyas elghoulti}, <oouhlale>{othman ouhlale}.*

# ft_irc

## Description

**ft_irc** is a project from the 42 curriculum that consists of building a simple Internet Relay Chat (IRC) server in C++.

The goal of this project is to understand how real-world network applications work by implementing a server that communicates using the IRC protocol. The server must be able to handle multiple clients simultaneously, manage channels, and support basic IRC commands.

Through this project, we explore:

* Socket programming (TCP/IP)
* Client-server architecture
* Multiplexing using `poll()`
* Parsing and command handling
* Concurrency and connection management

The final result is a functional IRC server that can be used with standard IRC clients.

---

## Instructions

### Compilation

To compile the project, run:

```bash
make
```

This will generate the executable (usually named `ircserv`).

### Usage

Run the server with:

```bash
./ircserv <port> <password>
```

* `<port>`: The port number the server will listen on
* `<password>`: The connection password required by clients

### Connecting to the Server

You can connect using an IRC client such as:

* HexChat
* weechat
* irssi

Example:

```bash
irssi -c 127.0.0.1 -p <port> -w <password>
```

---

## Features

* Multiple client connections handled efficiently with `poll()`
* Nickname and username management
* Channel creation and joining
* Message broadcasting
* Basic IRC commands:

  * `PASS`
  * `NICK`
  * `USER`
  * `JOIN`
  * `PART`
  * `PRIVMSG`
  * `MODE`
  * `QUIT`

---

## Technical Choices

* Language: C++
* Networking: BSD sockets (TCP)
* Multiplexing: `poll()`
* Architecture:

  * Event-driven server
  * File descriptor monitoring via `pollfd` structures
  * Command parser
  * Client and channel management structures

---

## Resources

### Documentation & References

* RFC 1459 - Internet Relay Chat Protocol
* Beej’s Guide to Network Programming
* Linux `man` pages (`socket`, `bind`, `listen`, `accept`, `poll`)
* C++ reference (cplusplus.com / cppreference.com)

### Tutorials & Articles

* Socket programming tutorials
* IRC protocol breakdowns
* `poll()` multiplexing explanations and comparisons with `select()`

### AI Usage

AI tools were used during the development of this project for:

* Understanding concepts such as `poll()` and event-driven servers
* Debugging networking issues and edge cases
* Refactoring and improving code structure
* Generating test scenarios and validating behavior

All architectural decisions and final implementation were written and validated manually.

---

## Notes

* This server implements a simplified version of the IRC protocol.
* Some advanced features from the official IRC specification may not be fully supported.
* The project focuses on correctness, robustness, and efficient handling of multiple clients.

---

## Authors

* <IEL-GHOU>
* <OOUHLALE>

