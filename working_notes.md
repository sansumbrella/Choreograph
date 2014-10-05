
TODO:

Sa: Bezier curve drawing sample to show off mix phrase (aka, crazy way to animate along a curve).
    Create a motion from each sub-phrase.
Sa: Loop Motion. Set time to zero on finish.
    PingPong Motion. * -1 speed on finish.
    Reverse Motion. Negative speed; set time to end.

Sequence editing: no removal once added, but can get slices of an existing sequence.
  Only thing that could mess up an existing motion (badly) would be to shorten the sequence while playing.

Retiming Phrase -> normalize input time, then ease it, then get underlying phrase value at new denormalized time.
  - Not so hot for simple Phrases, but retiming a whole Sequence might be cool.

More samples.
Graphical, Scratch-like animation creator/editor.
Doxyfile
