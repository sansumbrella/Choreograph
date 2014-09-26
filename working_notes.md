Virtual method cost is paid whenever we have a pointer to an object.
Copying the whole sequence into our motion generally gives a performance gain, though the shared sequence stuff doesn't run as fast any more.

## Most Performant

No virtual dispatch, ever.
Templatize each variation (Sequence<T, PhraseT>).
Always stack-allocate.

Performance is approximately as follows:
Working with 150000 motions.
[Creating Motions]                                70.905ms  
[Stepping Motions 1x]                             10.843ms  
[Stepping Motions 60x]                            667.836ms  
[Creating Motions from shared Sequence]           62ms  
[Stepping Motions with shared Sequence 1x]        11.651ms  
[Stepping Motions with shared Sequence 60x]       715.132ms

Issues:
Sometimes separating components and sometimes not requires two Sequences, can't be added at the same time.
Everything depends on the PhraseT.
Pseudo-instancing is gone, since we copy the Sequence to the Motion.
Makes mixing signals difficult.

## Most Flexible

Source<T> (virtual T getValue(time), non-virtual start and end times)
- Phrase inherits
	- Phrase2, Phrase3, etc. inherit (template on number of components?)
- Sequence inherits
	- list of sequential SourceRef<T>
- Mix inherits
	- list of concurrent SourceRef<T> with contribution factors
- safe and fast since we're calculating based on our internal states

TimelineItem (steppable)
	Motion (Maybe rename to Connection)
		SourceRef<T>
		OutputRef<T>
		playhead position
		begin/end/update callbacks
	Cue
		simple callback at time

TimeLine
	list of TimelineItemRefs

Issues:
Will have about a 15-20% performance hit relative to fastest path.
Granted, it can still drive 150k motions in under 16ms.
Also, having a shared_ptr to Sequence in Motion allows for pseudo-instancing,
which results in a nice performance boost when everyone does the same thing.

Performance is approximately as follows (measured with virtual Sequence type, most flexi not yet implemented):
Working with 150000 motions.
[Creating Motions]                                71.751ms  
[Stepping Motions 1x]                             13.117ms  
[Stepping Motions 60x]                            776.125ms		// virtual method dispatch slows us down a bit  
[Creating Motions from shared Sequence]           23.696ms		// copying only a pointer, hence speed here  
[Stepping Motions with shared Sequence 1x]        9.40001ms		// less memory traversal due to pseudo-instancing  
[Stepping Motions with shared Sequence 60x]       556.052ms

TODO:
Implement `Most Flexible` approach.
