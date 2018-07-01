# hot_wire_2_0
Arduino project for a "hot wire" game.

"hot wire" is a game of agility, featuring a strong wire that goes throug a wireloop. The wire is bent in a tricky shape and is mounted to a frame to hold it in the air. The wireloop has a handle.
The challange is to move the loop from one side of the wire to the other side without touching the wire.

Normally this is implemented very simple with a battery, a light and a buzzer. Loop and wire work as a switch, so touching the wire with the loop will light up the lamp and start the buzzer. When playing the game, there is a limit of touches until you have lost. 

This concept exists for decades an has some drawbacks I want to eliminate:

* if a short touch or two touches right after another is counted as a failure depends on the person supervising the game
* also, if a long touch may count as multple failures depends on the supervisor
* Counting touches is done manually
* Everybody nearby the game gets annoyed by the buzzer, when staying for longer (this normally hits the supervisor first)

Target of this project is a new "pimped" version of the game without loosing the primary challenge.

* Automatic detection and counting of touches
* Replace the buzzer with visual and haptic feedback to the player (Led's on the loop, Vibration motor in the handle)
* Posibility to select from multiple difficulties
* Clear and rewarding feedback for win/loss

To implement this, I chose to use an arduino uno.

