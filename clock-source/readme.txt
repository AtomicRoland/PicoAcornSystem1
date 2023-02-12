Assembly
========

You need beebasm to assemble this program. Run the build.sh script to assemble it. Syntax: ./build.sh clock
Several files will be generated. The clock.bin file is the assembled file. Clock.obj can be loaded directly into the memory of the System-1 emulator.

Running the program
===================

Load the program into memory from address &200
Start the program by pressing the G key followed by the address 0 2 0 0
Then press the G key again and the program will start

You will then see eight underscores on the displays. Now key in the time in 24-hour format. After the last digit has been entered, the eight dashes appear again. Now key in the date in the format:

Y Y Y Y M M D D → e.g. 2 0 2 3 0 2 1 2

After entering the date, the time is displayed again. Between the hours and minutes, the digital dot will turn on or off every second. You can display the date by pressing one of the up/down keys. The date remains until the minute has passed. You can reset the time with the S key.