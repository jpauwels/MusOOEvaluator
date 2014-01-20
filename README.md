MusOOEvaluator
==============

An evaluator for music label sequences based on the [MusOO] library

Compilation
-----------
MusOOEvaluator has only one dependency, the [boost] libraries (version >= 1.46.0). If it has been installed in a non-standard location, two environment variables have been defined such that no hard-coded paths need to be changed in the build systems. Just set `COMPILED_LIBRARIES_INCLUDE` and `COMPILED_LIBRARIES_LIB` to the include and library folders containing the boost libraries and you're good to go.

Build systems for multiple platforms are included in the `build` folder

* Mac OS X: XCode and GNU Make
* Linux: GNU Make
* Windows: Visual Studio 2008 and Visual Studio 2010

Additional build systems can be generated using [premake] and the `premake4.lua` file in the `build` directory.

Usage
-----
### Processing files or a list of files ###
The files to be evaluated can either be expressed as single files or as a list of files. Single file mode takes the options `--reffile <file-path>` and `--testfile <file-path>` to specify the path to the reference file and the file under test. For list mode, you need to create a text file with the base names (without extensions) of a data set separated by newlines. These base names can contain relative folders as well. You pass the path to this list file with `--list <list-path>`. Then you specify the reference and test directories that get prepended to the base names with `--refdir <dir-path>` and `--testdir <dir-path>` respectively, as well as the suffixes that get appended with `--refext <suffix>` and `--testext <suffix>`.

Both input modes can restrict the evaluation to a subsection of the files. This can be done globally for all files using the options `--begin <time>` or `--end <time>` or on a per file basis by passing a text file that contains the start and end times separated by whitespace. These timing files should be passed with the options `--timingfile <file-path>`, or `--timingdir <dir-path>` and `--timingext <suffix>`, depending on the input mode. The time is in all cases expressed in seconds. By default the evaluation is carried out over the entire duration of the reference file.

### Preset selection ###
Exactly how the two sequences of music labels will be compared depends on what they represent. Furthermore, for each type of label, a number of presets are defined that offer different ways of comparing the sequences. The music labels can either represent chords, keys or notes. The labels can be completely ignored as well and then an evaluation based just on the segmentation will be performed. For key labels, there is a special "global key" mode that first takes the key with the longest total duration in the file (in case there is more than one) before passing on to the evaluation. You select the desired content mode by specifying the correspoding option out of `--chords <preset-name>`, `--keys <preset-name>`, `--globalkey <preset-name>`, `--notes <preset-name>` or `--segmentation <preset-name>` on the command line.

###### Chords presets
* Triads
* Tetrads
* TriadsInput
* TetradsOnly
* Bass
* Root
* ChromaRecall
* ChromaPrecision
* ChromaFmeasure
* Mirex2009
* Mirex2010
* 4TriadsInput
* 4TriadsOutput
* 6TriadsInput
* 6TriadsOutput
* MirexMajMin
* MirexMajMinBass
* MirexSevenths
* MirexSeventhsBass
* MirexRoot

A description of the exact methods of comparison they represent can be found in the ICASSP[^1] paper on chord evaluation and the MIREX website[^2].

###### Keys and globalkey presets
* Mirex: evaluation according to the rules of [MIREX 2006]
* Binary: only an exact match (or its enharmonical equivalent) is considered correct
* Lerdahl: answering a related key gives a fraction of the score, depending on its regional distance according to Lerdahl[^3]

###### Notes
Currently there is only one preset, `Piano`, available which compares notes in the piano range (MIDI notes 21 to 89) and considers them correct if they are enharmonically  equal to the reference note.

###### Segmentation
* Onset
* Offset
* Inner
* Outer

These presets are referring to a work in process that does not write output yet. For now, all of them return the same directional Hamming measure and over-segmentaton and under-segmentation values.

###### Presets used in publications
The exact options that were used for the 2013 ICCASP paper[^1] are `--chords Mirex2010`, `--chords Triads`, `--chords Tetrads`, `--chords TriadsInput`, `--chords TetradsOnly`, `--chords Root`, `--chords Bass`, `--chords ChromaRecall`, `--chords ChromaPrecision`.  
The exact options that were used in the MIREX 2013 ACE[^2] evaluation campaign were `--chords MirexMajMin`, `--chords MirexMajMinBass`, `--chords MirexSevenths`, `--chords MirexSeventhsBass`, `--chords MirexRoots` and `--segmentation Inner`.

### Output options ###
A result file with a score for the entire data set specified as input is written to the path given by `--output <file-path>`. Its output is a human readable summary that depends on the content mode, but if you add the option `--csv`, you get a comma separated value file in the same directory as the output file with separate results for each of the input files. For the "chords", "keys", "globalkey" and "notes" content modes, you can also get a confusion matrix by specifying `--confusion <path>`. Finally, for these same content modes, you can specify the `--verbose` option to get an idea what exactly is going on. It will write a file per input in the output directory in which the different steps of the score calculation are broken down. All the reference-test segment pairs are enumerated, with original labels, mapped labels, segment duration and whether or it is included in the evaluation. This gives a sort of "paper trail" that shows how the score is calculated so that it is easy to verify by hand and understand what exactly is going on.

[^1]: [Johan Pauwels and Geoffroy Peeters, *Evaluating automatically estimated chord sequences*, Proceedings of the IEEE International Conference on Audio, Speech and Signal Processing (ICASSP), 2013.](http://dx.doi.org/10.1109/ICASSP.2013.6637748)
[^2]: [MIREX 2013 Automatic Chord Estimation task](http://www.music-ir.org/mirex/wiki/2013:Audio_Chord_Estimation)
[^3]: [Fred Lerdahl, *Tonal pitch space*, Oxford University Press, 2001](http://dx.doi.org/10.1093/acprof:oso/9780195178296.001.0001)

[MusOO]: https://github.com/jpauwels/libMusOO
[boost]: http://www.boost.org
[premake]: http://industriousone.com/premake
[MIREX 2006]: http://www.music-ir.org/mirex/wiki/2005:Audio_and_Symbolic_Key_Finding#Evaluation_Procedures