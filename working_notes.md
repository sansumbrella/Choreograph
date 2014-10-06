
TODO:

MotionGroup for start/finish functions over a collection of Motions.
 - Timeline::collect( MotionRef... );
 - MotionGroup::finishFn, ::startFn
 - Enable repeating of the motion over several parameters.
 - Consider using for Bezier Construction sample, since they are all related.
 - duration = reduce( motions, max( m.endTime ) )

Sequence editing: no removal once added, but can get slices of an existing sequence.
  Only thing that could mess up an existing motion (badly) would be to shorten the sequence while playing.

Retiming Phrase -> normalize input time, then ease it, then get underlying phrase value at new denormalized time.
  - Fairly pointless for simple Phrases, but retiming a whole Sequence might be cool.

More samples.
Graphical, Scratch-like animation creator/editor.
Doxyfile
