# ntetris
A tetris clone that uses the [ncurses](https://en.wikipedia.org/wiki/Ncurses) API.

Supported OS: Linux

## Installation

1. Install ncurses. For example, using `apt-get`:

`sudo apt-get install libncurses5-dev`

2. Clone this repository. For example:

`git clone https://github.com/AYWG/ntetris.git`

3. Change into the *src* directory, type `make`, and press Enter.

4. To play the game, type `ntetris` and press Enter (from any directory).

5. Enjoy!

## Usage and Instructions

#### Usage

`ntetris [--option]`

The following options are available:

`about` : Displays version and developer information

`help` : Displays a help message

`howtoplay` : Displays instructions on how to play ntetris

`stats` : Runs the game; if player exits during or immediately after losing a single player game, the player's stats are shown on the console.

#### How to play (Single)

- Control the falling tetriminos by moving or rotating them inside the well.
- Complete horizontal lines to clear them.
- The more lines you clear at once, the more points you earn!
- The game ends when the tetriminos stack up to the top of the well.

#### How to play (Versus)

- Control the falling tetriminos by moving or rotating them inside your well.
- Complete horizontal lines to clear them.
- The more lines you clear at once, the more garbage lines you can add to your opponent's well, and the more garbage lines you can take away from your well!
- The first player whose tetriminos stack up to the top of their well loses.

#### Controls

                                          Single/Player 1      Player 2

    Move tetrimino left                   Left arrow key       A

    Move tetrimino right                  Right arrow key      D

    Move tetrimino down                   Down arrow key       S

    Drop tetrimino                        Up arrow key         W      

    Rotate tetrimino clockwise            P                    G     

    Rotate tetrimino counterclockwise     O                    F

    Hold tetrimino                        Enter                Space                           

## Screenshots

<img src="https://github.com/AYWG/ntetris/blob/master/img/ntetris_menu.jpg" alt="ntetris menu" width="420" height="300" >
<img src="https://github.com/AYWG/ntetris/blob/master/img/ntetris_single.jpg" alt="ntetris single" width="420" height="300" >
<img src="https://github.com/AYWG/ntetris/blob/master/img/ntetris_versus.jpg" alt="ntetris versus" width="420" height="300">


