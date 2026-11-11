## HOTDOG FEST
A simple 2D game made with Raylib. You play as dachshunds who compete in short rounds of chaotic mini-games.

Dogs can run, jump, dig and bark through levels full of objects that allow for crazy physics interactions and chain reactions. Also, slopes. Those were a pain to implement.

The game is still in development. It is probably the fourth iteration of the game, but I have high hopes for this one.

It will also feature online multiplayer, or the best approximation thereof.

## Expected workflow:
- [X] Add level background.
- [X] Add dog stun mechanic.
- [X] Add dog grabbing and stealing mechanic.
- [X] Add dog barking mechanic.
- [X] Add dog trampoline bouncing mechanic.
- [X] Add terrain digging mechanic.
- [ ] Physics improvements.
- [ ] Add water physics and visuals.
- [ ] Networking improvements.
- [ ] Proper level saving and loading.
- [ ] Sound effects.
- [ ] Particles.

## Non-obvious to-do list (Things I could very well forget):
- [ ] Fix dynamic objects clipping through slopes with solid roof (47, 48)
- [ ] Fix dog stacked on dog not counting as grounded.
- [ ] Add swallowed check on slope sweeping in X axis. (Is it necessary?)
- [ ] Determine how digging priorities are handled.
- [ ] Add digging in centerX as a fail-safe option in case nothing is found in left or right.
- [ ] Decide if a stunned dog that is stunned by another dog can change its interaction cause and depth.
- [X] Specialize interaction velocity.
- [X] Separate interaction velocity in X and Y components.
- [ ] Improve force field feel.
- [X] Make the dog an interaction agent.
- [ ] Add in-terrain physics interactions.
- [ ] Entities must detect partial semisolids when exiting terrain.
- [ ] Add fast moving dog stunning.
- [ ] Add fast moving ball stunning.
- [ ] Physics act differently in some devices. Unsure if it's compiler or system stuff.
- [ ] Properly poll editor input.
- [ ] Add dog out-of-bounds respawning.
