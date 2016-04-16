---
KDevelop Integration with Arduino and embedded development
    ------------------------------------------------------------------------

    Electronic engineering, Federal University of Santa Catarina, Brazil,
    e-mail: <patrickelectric at gmail dot com>


Motivation
==========

The actual Arduino IDE, was initially created with
Java and is still a simple IDE that does
not provides autocompletion, sentence errors, assembly visualizer, field
for compiling and linking flags, syntax highlighting and other features
present in both [Kdevelop](https://www.kdevelop.org/) and [Qt
Creator](http://www.qt.io/ide/).

The other alternative IDEs for Arduino are:

-   PROGRAMINO IDE <br/>
    Price: 30�€� <br/>
    It provides good features and development options, but isn’t a free
    or open source software.

-   PlatformIO IDE <br/>
    Price: FREE <br/>
    A good substitute to Arduino IDE. It’s a modified
    [Atom](https://atom.io/) version, Doesn’t have assembly
    visualization, serial plotting, data loggging and other
    important tools. It’s written in CoffeeScript, JavaScript, Less
    and HTML.

-   Embrio <br/>
    Price: $50 <br/>
    An easy variable visualization ambient with “real-time" plot with
    code editor. But, isn’t free or open source.

Some developers and [educators](http://www.hackvandedam.nl/blog/?p=762)
say that Arduino IDE isn’t as good and comfortable as a development
ambient for higher education and development. In order to the to fix
this situations, the development of Arduino plugins for KDevelop, Qt
creator, Visual Studio and Eclipse began, but generally the setup for
such plugins is complicated and it’s necessary a good understanding of
these IDEs and OS’s ambient functionality.

Project goals
=============

Here I will showcase some of the features that I propose to implement.
Right now I’m the [second biggest
contributor](https://github.com/mupuf/arduide/graphs/contributors) of a
project called [ArduIDE](http://mupuf.org/project/arduide.html), a C++
IDE to help with software development to Arduino boards.

The initial idea is to work with KDevelop to port some implemented features that
already exist in others embedded systems IDE, like:
memory map view, assembly visualizer and others.

Some of the goals are:

1.  Port Arduino support from 1.6.0 to last Arduino version
    into KDevelop.

2.  Port and create others features like: <br/> - Board selector. <br/> - Clock
    selector. <br/> - Serial monitor. <br/> - Assembly visualization. <br/> - Compilation
    flags editor. <br/> - Real-time plotting with serial input, with
    integrator and derivator plot. <br/> - Real-time data logging.

3.  Perform the upload process for Arduino boards with AVR processors.

4.  Perform the upload process for Arduino boards with ARM processors.

5.  (If time isn’t a problem) Realize the upload process for Arduino
    boards with x86 processors.

    With all Arduino features implemented, the development of ARM
    processors can start:

6.  Implement same features of Arduino development with ARM processors.

7.  Add support to JTAGs(Joint Test Action Group) with [OpenOCD](http://openocd.org/).

8.  Add GDB debugger with OpenOCD interface.

Timeline
========

This section shows the estimated timeline of the project.

### Done:
#### Arduino
  - Download and install tools.


May:
----

Initial week, change ArduIDE features from Qt4 to Qt5 in KDevelop.
Almost everything in ArduIDE is done with Qt4,
[Grantlee](https://github.com/steveire/grantlee) and
[QScintilla](https://riverbankcomputing.com/software/qscintilla/intro)

It’s possible to replace grantlee with QML since ArduIDE create only a
dynamic webpage with examples and libraries installed on the system.
Right now a good part of ArduIDE it’s in Qt5 in my personal repository
but isn’t finished yet.

Some of the features that already exist in ArduIDE that will be ported
to KDvelop:

-   Board selector

-   Clock selector

-   Serial monitor

-   Assembly visualization

-   Compilation flags

June:
-----

After the transition from Qt4 to Qt5, the IDE needs to be updated to the
last version of Arduino software and be compatible with the boards.

Test the software compatibility and programmability of KDevelop with
Arduino’s source code and libraries. After that, a plugin will be
created to manage what kind of embedded system the project will use
during the development, this same plugin will manage other plugins that
will provide a variable system development. </br>
In the beginning will be only one children plugin to manage the Arduino
environment and process. To perform such boards and processors
management it’s possible to use a generic *makefile* to compile and
upload the project to Arduino boards. </br>
With selectors menu, like Board and processor clock, it’s possible to
use the Kdevelop module called *Sublime* with a drop-down menu in
toolbar that will be display the options to the user, and with this
information it’s possible to modify a generic Arduino makefile to manage
the project compiling and upload process.

The data-log can be added in a Dock, the same back-end of data-log can
be used to the a plot system that will provide in the same Dock a
user visualization tool to display the output data of the board.

July:
-----

With all Arduino AVR boards working, the process of Arduino ARM boards
begun. Unlike the AVR boards which use
[*avrdude*](http://www.nongnu.org/avrdude/), the ARM boars use
[*bossac*](http://www.shumatech.com/web/products/bossa) to perform the
upload process. The same process of Arduino AVR can be used with the
Arduino ARM boards, using a generic makefile, but with *bossac* and
*arm-none-eabi-binutils* in the place of *avrdude*. </br>
The Arduino X86 boards uses a script to perform the compile and upload
process. If time isn’t a problem until now in the project the
implementation of such feature will be performed. </br>
With *arm-none-eabi-binutils* we already have a .bin, .elf or .hex to be
uploaded to an ARM processor. It’s necessary something between the
processor and the computer to realize the upload process, this thing
it’s called JTAG or programmer interface, can it be a Pirate BUS, JTAG,
DTAG and ICSP interface or a simple FTDI232R/H to realize the midfield
to the in-chip JTAG. It’s possible to communicate with all this
interfaces with OpenOCD, a list with all possible interfaces can be
located
[here](http://openocd.org/doc/html/Debug-Adapter-Configuration.html).
Using OpenOCD can be a bit complicated but not impossible !

August:
-------

If the last point finish with success, this part can be done without so
much problems. With a OpenOCD link with the board interface, it’s
possible to use the GDB with a remote access to the socket door that
OpenOCD open to realize the communication with GDB and the hardware
components of the processor debug. </br>
With all this steps done, we have created a awesome embedded system
platform with our friend KDevelop :) </br>
The last week will be reserved to solve some bugs and finish some points
in the documentation of the project.
