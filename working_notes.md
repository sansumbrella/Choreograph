
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
Copy Phrases from Sequence when doing continuations.
Generally immutable data (for parts the sequence cares about).

What if Sequences were not Phrases?
- immutable Phrase duration becomes consistent rule.
- no need for friend declaration.
- CombinePhrase couldn't wrap Sequences if it's wrapping Phrases
- Phrase decorator could compose a Sequence (it might need a note not to change the Sequence, or it could copy the Sequence)
	- allowing us to then put the sequence into a CombinePhrase.
- Sequence::makePhrase() wraps Sequence in the above-mentioned Phrase decorator.

Phrases are primarily immutable motions.
Certain phrases, like Mix and Combine, have a small mutable interface.
