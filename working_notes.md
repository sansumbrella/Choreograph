
Since Sources now only have a duration, nothing really surprising will happen if they are shared between multiple
sequences. You might be surprised to have a change to the duration of a phrase propagate throughout all your sequences,
but the effect of sharing should be obvious. You can always make a need shared_ptr copy to avoid the sharing.

The atomic increment cost of shared_ptr probably isn’t any higher than copying an entire object, especially when adding
Sequences to Sequences.

Next step:
Sequences to hold shared_ptr’s to their underlying Phrases.
Remove the clone() method from Phrase.
Rename Source<T> to Phrase<T> (probably).
Don't let people change duration.
Instead, easier insert/remove phrases from sequence. Just create a new one with the duration you want.
Copy phrases from Sequence when doing continuations.
Generally immutable data (for parts the sequence cares about).
