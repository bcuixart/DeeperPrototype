## HOTDOG FEST
A simple 2D game made with Raylib. You play as dachshunds who compete in short rounds of chaotic mini-games.

Dogs can run, jump, dig and bark through levels full of objects that allow for crazy physics interactions and chain reactions. Also, slopes. Those were a pain to implement.

The game is still in development. It is probably the fourth iteration of the game, but I have high hopes for this one.

It will also feature online multiplayer, or the best approximation thereof.

## Non-obvious to-do list (Things I could very well forget):
- [ ] Fix dynamic objects clipping through slopes with solid roof (47, 48)
- [ ] Add swallowed check on slope sweeping in X axis. (Is it necessary?)
- [ ] Determine how digging priorities are handled.
- [ ] Add digging in centerX as a fail-safe option in case nothing is found in left or right.
- [X] Specialize interaction velocity.
- [ ] Separate interaction velocity in X and Y components.
- [ ] Improve force field feel.
- [ ] Make the dog an interaction agent.
- [ ] Physics act differently in some devices. Unsure if it's compiler or system stuff.
- [ ] Properly poll editor input.
