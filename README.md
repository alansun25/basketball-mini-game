# Basketball Mini-Game

Final project for CSCI 371: Computer Graphics, Fall 2022.

Game code in `hw.cpp`.

## Game

This repository contains code for a basketball mini-game where the goal is to make as many shots as possible in the allotted timeframe. To run the game, type `source mac_build_and_run.sh` in the terminal.

![Gameplay GIF](https://github.com/alansun25/basketball-mini-game/blob/main/gameplay.gif)
(The GIF is slowed down for some reason.)

### Features
- User clicks to start shot, drags to set trajectory and force in desired direction, and releases to execute the shot.
- Trajectory visualized as the user drags and adjusts their shot.
- Collision detection between the ball and various components of the hoop (can toggle on/off collision geometry).
- Ball resets to original position when it hits the ground so the user can immediately shoot again.
- Score tracking for current round. High-score tracker persistent across all sessions of the game on a single machine.
- Round starts when first shot is released and ends after 30 seconds, with a timer displayed at the top of the screen.
- Can flip through different backgrounds for the game.
- Can toggle on "practice mode", which removes the timer and score trackers.
- Includes a "reset game" button to start over.

## Acknowledgements

- Special thanks to Jim Bern for starter code and graphics API.
- I implemented collision detection and resolution by modifying the algorithms described [here](https://learnopengl.com/In-Practice/2D-Game/Collisions/Collision-detection) and [here](https://learnopengl.com/In-Practice/2D-Game/Collisions/Collision-resolution).
- Background art sources:
    - [Grassy](https://www.wallpaperflare.com/pixel-art-8-bit-sky-beauty-in-nature-plant-environment-wallpaper-cpylb)
    - [Cityscape](https://wallpaperaccess.com/8-bit-aesthetic)
    - [Volcano](https://www.artstation.com/artwork/qAXJLz)
    - [Space](https://www.freepik.com/free-vector/pixel-art-mystical-background_29019077.htm#query=pixel%20moon&position=0&from_view=keyword)
    - [Vaporwave](https://wallpaperaccess.com/8-bit-aesthetic)
