//============================================================================
/**	@file

	.
	
	@author		Johan Pauwels
	@date		20100913
*/
//============================================================================

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <stdexcept>
#include <ctime>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/program_options.hpp>
#include <Eigen/Core>
#include "MusOOFile/KeyFileUtil.h"
#include "MusOOFile/ChordFileUtil.h"
#include "MusOOFile/NoteFileMaps.h"
#include "PairwiseEvaluation.h"
#include "KeyEvaluationStats.h"
#include "ChordEvaluationStats.h"
#include "NoteEvaluationStats.h"
#include "SimilarityScoreKey.h"
#include "SimilarityScoreChord.h"
#include "SimilarityScoreNote.h"
#include "SegmentationEvaluation.h"

using std::cout;
using std::cerr;
using std::endl;
using std::fixed;
using std::setw;
using std::setprecision;
using std::string;
using std::vector;
using std::exception;
using std::invalid_argument;
using std::runtime_error;
using boost::filesystem::path;
using boost::filesystem::ifstream;
using boost::filesystem::ofstream;
using boost::program_options::options_description;
using boost::program_options::value;
using boost::program_options::variables_map;
using boost::program_options::parse_command_line;
using boost::program_options::store;
using boost::program_options::notify;
using namespace MusOO;

void parseCommandLine(int inNumOfArguments, char* inArguments[], path& outOutputFilePath, path& outListPath,
	path& outRefPath, path& outTestPath, string& outRefExt, string& outTestExt,
	double& outBegin, double& outEnd, double& outTimeDelay, variables_map& outVarMap)
{
	options_description theGeneralOptions("General options");
	theGeneralOptions.add_options()
		("help,h", "produce this help message")
		("output", value<path>(&outOutputFilePath), "path to the output file")
		("chords", value<string>(), "select chords mode")
		("keys", value<string>(), "select keys mode")
		("globalkey", value<string>(), "select global key mode")
        ("notes", value<string>(), "select notes mode")
        ("segmentation", value<string>(), "select segmentation mode")
		("begin", value<double>(&outBegin)->default_value(0.), "the start time in seconds")
		("end", value<double>(&outEnd)->default_value(0., "file end"), "the end time in seconds")
		("csv", "Print results for individual files to file in comma separated format")
		("delay", value<double>(&outTimeDelay)->default_value(0.), "Add a time delay to the files to evaluate")
        ("confusion", value<path>(), "path to resulting global confusion matrix")
        ("verbose", "Write comparison file for each individual file")
		;

	options_description theRelativeListOptions("Relative list options");
	theRelativeListOptions.add_options()
		("list", value<path>(&outListPath), "list with base file names")
        ("refdir", value<path>(&outRefPath), "directory with reference annotations")
        ("refext", value<string>(&outRefExt), "extension of the reference annotations")
		("testdir", value<path>(&outTestPath), "directory with files to evaluate")
		("testext", value<string>(&outTestExt), "extension of the files to evaluate")
		("timingdir", value<path>(), "directory with files with start and end times")
		("timingext", value<string>()->default_value("-timing.txt"), "extension of the files with start and end times")
		;
    
	options_description theSingleFileOptions("Single file options");
	theSingleFileOptions.add_options()
        ("reffile", value<path>(&outRefPath), "file with reference annotation")
        ("testfile", value<path>(&outTestPath), "file to evaluate")
        ("timingfile", value<path>(), "file with start and end times")
        ;

	options_description theOptions;
	theOptions.add(theGeneralOptions).add(theRelativeListOptions).add(theSingleFileOptions);

	store(parse_command_line(inNumOfArguments, inArguments, theOptions), outVarMap);
	notify(outVarMap);

	if (inNumOfArguments < 2 ||outVarMap.count("help") > 0)
	{
		cout << "Description: This application compares music label sequences.\n"
			<< "    The reference and test sequence(s) can be specified by two modes:\n"
			<< "    as single files or as a list of multiple files.\n" << endl;
		cout << theOptions << endl;
		cout << "Author: Johan Pauwels - Build: " << __DATE__ << endl;
		cout << endl;
		exit(0);
	}
    
	if (outVarMap.count("chords") + outVarMap.count("keys") + outVarMap.count("globalkey") + outVarMap.count("notes") + outVarMap.count("segmentation") != 1)
	{
		throw invalid_argument("Please select a single 'chords', 'keys', 'globalkey', 'notes' or 'segmentation' mode");
	}
	if (!exists(absolute(outOutputFilePath.parent_path())))
	{
		create_directories(outOutputFilePath.parent_path());
	}
    
	if (outVarMap.count("list") > 0)
    {
        // One list mode
        if (!is_regular_file(outListPath))
        {
            throw invalid_argument(outListPath.string() + " is not an existing file");
        }
        if (outVarMap.count("refdir") > 0)
        {
             if(!is_directory(outRefPath))
            {
                throw invalid_argument(outRefPath.string() + " is not an existing directory");
            }
        }
        else
        {
            throw invalid_argument("Please specifiy a reference directory");
        }
        if (outVarMap.count("testdir") > 0)
        {
            if (!is_directory(outTestPath))
            {
                throw invalid_argument(outTestPath.string() + " is not an existing directory");
            }
        }
        else
        {
            throw invalid_argument("Please specifiy a test directory");
        }
        if (outVarMap.count("timingdir") > 0 && !is_directory(outVarMap["timingdir"].as<path>()))
        {
            throw invalid_argument(outVarMap["timingdir"].as<path>().string() + " is not an existing directory");
        }
    }
    else
    {
        // Single file mode
        if (outVarMap.count("reffile") > 0)
        {
            if (!is_regular_file(outRefPath))
            {
                throw invalid_argument(outRefPath.string() + " is not an existing file");
            }
        }
        else
        {
            throw invalid_argument("Please specifiy a reference file");
        }
        if (outVarMap.count("testfile") > 0)
        {
            if (!is_regular_file(outTestPath))
            {
                throw invalid_argument(outTestPath.string() + " is not an existing file");
            }
        }
        else
        {
            throw invalid_argument("Please specifiy a test file");
        }
        if (outVarMap.count("timingfile") > 0 && !is_regular_file(outVarMap["timingfile"].as<path>()))
        {
            throw invalid_argument(outVarMap["timingfile"].as<path>().string() + " is not an existing file");
        }
    }
}

const std::vector<std::string> readList(const path& inListPath)
{
    if (inListPath.empty())
    {
        return vector<string>(1, "");
    }
    else
    {
        vector<string> theListItems;
        ifstream theListIn(inListPath);

        if(!theListIn.is_open())
        {
            throw runtime_error("Could not open list " + inListPath.string());
        }
        //run over all files in the list
        string theListItem;
        while(getline(theListIn, theListItem))
        {
            // handle windows files under unix
            if (theListItem[theListItem.size()-1] == '\r')
            {
                theListItem.erase(theListItem.size()-1);
            }
            theListItems.push_back(theListItem);
        }
        return theListItems;
    }
}

void printConfusionMatrix(std::ostream& inOutputStream, const Eigen::ArrayXXd& inConfusionMatrix,
						  const std::vector<std::string>& inLabels, const string inSeparator = ",", const string inQuote = "\"")
{
	inOutputStream << inSeparator << inQuote;
	copy(inLabels.begin(), inLabels.end()-1, std::ostream_iterator<string>(inOutputStream, (inQuote+inSeparator+inQuote).c_str()));
	inOutputStream << inLabels.back() << inQuote << endl;
	for (int i = 0; i < inConfusionMatrix.rows(); ++i)
	{
		inOutputStream << inQuote << inLabels[i] << inQuote;
		for (int j = 0; j < inConfusionMatrix.cols(); ++j)
		{
			inOutputStream << inSeparator << fixed << setprecision(9) << inConfusionMatrix(i,j);
		}
		inOutputStream << endl;
	}
}

Key findLongestKey(TimedKeySequence inKeySequence)
{
	vector<std::pair<Key,double> > theUniqueKeys;
	//	std::unordered_map<Key,double> theUniqueKeys;
	for (TimedKeySequence::const_iterator i = inKeySequence.begin(); i != inKeySequence.end(); ++i)
	{
		//		theUniqueKeys[i->label()] += i->offset()-i->onset();
		bool theKeyFound = false;
		for (size_t j = 0; j < theUniqueKeys.size(); ++j)
		{
			if (i->label() == theUniqueKeys[j].first)
			{
				theUniqueKeys[j].second += i->offset()-i->onset();
				theKeyFound = true;
			}
		}
		if (!theKeyFound)
		{
			theUniqueKeys.push_back(std::pair<Key,double>(i->label(), i->offset()-i->onset()));
		}
	}
	Key theLongestKey = theUniqueKeys.front().first;
	double theDuration = theUniqueKeys.front().second;
	for (size_t j = 1; j < theUniqueKeys.size(); ++j)
	{
		if (theUniqueKeys[j].second > theDuration)
		{
			theLongestKey = theUniqueKeys[j].first;
			theDuration = theUniqueKeys[j].second;
		}
	}
	return theLongestKey;
}

void constructPaths(const std::string& inBaseName, const path& theRefDirName, const std::string& theRefExt, const path& theTestDirName, const std::string& theTestExt, const variables_map& theVarMap, path& outRefFileName, path& outTestFileName, double& outBegin, double& outEnd)
{
    outTestFileName = theTestDirName / path(inBaseName + theTestExt);
    outRefFileName = theRefDirName / path(inBaseName + theRefExt);
    if (theVarMap.count("timingdir") > 0 || theVarMap.count("timingfile") > 0)
    {
        path theTimingPath;
        if (theVarMap.count("timingdir") > 0)
        {
            theTimingPath = theVarMap["timingdir"].as<path>() / path(inBaseName + theVarMap["timingext"].as<string>());
        }
        else
        {
            theTimingPath = theVarMap["timingfile"].as<path>();
        }
        boost::filesystem::ifstream theTimingFile(theTimingPath);
        if (!theTimingFile.is_open())
        {
            throw runtime_error("Could not open file " + theTimingPath.string() + " for reading.");
        }
        theTimingFile >> outBegin >> outEnd;
    }
}

/**	Start point of the program. */
int main(int inNumOfArguments,char* inArguments[])
{ try {

	path theOutputPath;
	path theListPath;
	path theTestDirName;
	path theRefDirName;
	string theTestExt;
	string theRefExt;
	double theBegin;
	double theEnd;
	double theDelay;
	variables_map theVarMap;

	parseCommandLine(inNumOfArguments, inArguments, theOutputPath, theListPath,
		theRefDirName, theTestDirName, theRefExt, theTestExt,
		theBegin, theEnd, theDelay, theVarMap);

	string theCSVSeparator = ",";
    string theCSVQuotes = "\"";
    
    path theRefPath;
    path theTestPath;
	double theTotalDuration = 0.;
	double theWeightedScore = 0.;
	Eigen::ArrayXXd theGlobalConfusionMatrix;
	vector<string> theLabels;
    
	vector<string> theListItems = readList(theListPath);
    
	ofstream theOutputFile(theOutputPath);
	if (!theOutputFile.is_open())
	{
		throw runtime_error("Could not open output file " + theOutputPath.string());
	}
	theOutputFile << "List: " << theListPath << "\n" << endl;

	/**********************/
	/* Keys or global key */
	/**********************/
	if (theVarMap.count("keys") > 0 || theVarMap.count("globalkey") > 0)
	{
		PairwiseEvaluation<Key>* theKeyEvaluation = NULL;
		if (theVarMap.count("keys") > 0)
		{
			theKeyEvaluation = new PairwiseEvaluation<Key>(theVarMap["keys"].as<string>());
		}
		else
		{
			theKeyEvaluation = new PairwiseEvaluation<Key>(theVarMap["globalkey"].as<string>());
		}
		theGlobalConfusionMatrix = 
			Eigen::ArrayXXd::Zero(theKeyEvaluation->getNumOfLabels(),
			theKeyEvaluation->getNumOfLabels());
		theLabels = theKeyEvaluation->getLabels();

		ofstream theCSVFile;
		if (theVarMap.count("csv") > 0)
		{
			theCSVFile.open(path(theOutputPath).replace_extension("csv"));
			if (theVarMap.count("keys") > 0)
			{
				theCSVFile << theVarMap["keys"].as<string>() << endl;
				theCSVFile << "File" << theCSVSeparator << "Pairwise score (%)"
					<< theCSVSeparator << "Duration (s)"
					<< theCSVSeparator << "Unique ref keys"
					<< theCSVSeparator << "Unique test keys" << endl;
			}
			else
			{
				theCSVFile << theVarMap["globalkey"].as<string>() << endl;
				theCSVFile << "File" 
					<< theCSVSeparator << "Score" 
					<< theCSVSeparator << "Ref key" 
					<< theCSVSeparator << "Test key" << endl;
			}
			theCSVFile << std::fixed;
		}

		for (vector<string>::const_iterator i = theListItems.begin(); i != theListItems.end(); ++i)
		{
			cout << "Evaluating file " << *i << endl;
            constructPaths(*i, theRefDirName, theRefExt, theTestDirName, theTestExt, theVarMap, theRefPath, theTestPath, theBegin, theEnd);
            
            TimedKeySequence theRefKeys = KeyFileUtil::readKeySequenceFromFile(theRefPath, true);
			TimedKeySequence theTestKeys = KeyFileUtil::readKeySequenceFromFile(theTestPath, false);
            
			if (theVarMap.count("keys") > 0)
			{
                ofstream theVerboseStream;
                if (theVarMap.count("verbose"))
                {
                    path theVerbosePath = theOutputPath.parent_path() / path(*i + ".csv");
                    theVerboseStream.open(theVerbosePath);
                }
				theKeyEvaluation->evaluate(theRefKeys, theTestKeys, theBegin, theEnd, theVerboseStream, theDelay);
				double theDuration = theKeyEvaluation->getTotalDuration();
				theTotalDuration += theDuration;
				theWeightedScore += theKeyEvaluation->getOverlapScore() * theKeyEvaluation->getTotalDuration();
				Eigen::ArrayXXd theConfusionMatrix = theKeyEvaluation->getConfusionMatrix();
				theGlobalConfusionMatrix += theConfusionMatrix;

				if (theVarMap.count("csv") > 0)
				{
					KeyEvaluationStats theStats(theConfusionMatrix);
					theCSVFile << theCSVQuotes << *i << theCSVQuotes << theCSVSeparator 
						<< 100*theKeyEvaluation->getOverlapScore() << theCSVSeparator
						<< theDuration << theCSVSeparator
						<< theStats.getNumOfUniquesInRef() << theCSVSeparator
						<< theStats.getNumOfUniquesInTest() << endl;
				}
			}
			else
			{
				SimilarityScoreKey theSimilarityScoreKey(theVarMap["globalkey"].as<string>());
				Key theGlobalRefKey = findLongestKey(theRefKeys);
				Key theGlobalTestKey = findLongestKey(theTestKeys);
				double theScore = theSimilarityScoreKey.score(theGlobalRefKey, theGlobalTestKey);
				theWeightedScore += theScore;
				++theTotalDuration;
				++theGlobalConfusionMatrix(theSimilarityScoreKey.getTestIndex(),
					theSimilarityScoreKey.getRefIndex());
				if (theVarMap.count("csv") > 0)
				{
					theCSVFile << theCSVQuotes << *i << theCSVQuotes << theCSVSeparator 
						<< theScore << theCSVSeparator
						<< KeyElis(theGlobalRefKey).str() << theCSVSeparator
						<< KeyElis(theGlobalTestKey).str() << endl;
				}
			}
 		}
		delete theKeyEvaluation;
	}
	/**********/
	/* Chords */
	/**********/
	else if (theVarMap.count("chords") > 0)
	{
		PairwiseEvaluation<Chord> theChordEvaluation(theVarMap["chords"].as<string>());
		theGlobalConfusionMatrix = Eigen::ArrayXXd::Zero(theChordEvaluation.getNumOfLabels(),
			theChordEvaluation.getNumOfLabels());
		theLabels = theChordEvaluation.getLabels();

		ofstream theCSVFile;
		if (theVarMap.count("csv") > 0)
		{
			theCSVFile.open(path(theOutputPath).replace_extension("csv"));
			theCSVFile << theVarMap["chords"].as<string>() << endl;
			theCSVFile << "File" 
				<< theCSVSeparator << "Pairwise score (%)"
				<< theCSVSeparator << "Duration (s)" 
				<< theCSVSeparator << "All correct (%)" 
				<< theCSVSeparator << "Root correct (%)" 
				<< theCSVSeparator << "Type correct (%)" 
				<< theCSVSeparator << "All wrong (%)"
				<< theCSVSeparator << "Unique ref chords"
				<< theCSVSeparator << "Unique test chords" << endl;
			theCSVFile << std::fixed;
		}

		for (vector<string>::const_iterator i = theListItems.begin(); i != theListItems.end(); ++i)
		{
			cout << "Evaluating file " << *i << endl;
            constructPaths(*i, theRefDirName, theRefExt, theTestDirName, theTestExt, theVarMap, theRefPath, theTestPath, theBegin, theEnd);
            
			TimedChordSequence theRefChords = ChordFileUtil::readChordSequenceFromFile(theRefPath, true);
            TimedChordSequence theTestChords = ChordFileUtil::readChordSequenceFromFile(theTestPath, false);

            ofstream theVerboseStream;
            if (theVarMap.count("verbose"))
            {
                path theVerbosePath = theOutputPath.parent_path() / path(*i + ".csv");
                theVerboseStream.open(theVerbosePath);
                theVerboseStream << "Start" << theCSVSeparator << "End" << theCSVSeparator << "RefLabel" << theCSVSeparator << "TestLabel" << theCSVSeparator << "Score" << theCSVSeparator << "Duration" << theCSVSeparator << "MappedRefLabel" << theCSVSeparator << "MappedTestLabel" << theCSVSeparator << "RefChromas" << theCSVSeparator << "TestChromas" << theCSVSeparator << "NumCommonChromas" << theCSVSeparator << "RefBass" << theCSVSeparator << "TestBass" << endl;
            }
			theChordEvaluation.evaluate(theRefChords, theTestChords, theBegin, theEnd, theVerboseStream, theDelay);
			double theDuration = theChordEvaluation.getTotalDuration();
			theTotalDuration += theDuration;
			theWeightedScore += theChordEvaluation.getOverlapScore() * theDuration;
			Eigen::ArrayXXd theConfusionMatrix = theChordEvaluation.getConfusionMatrix();
			theGlobalConfusionMatrix += theConfusionMatrix;

			
			if (theVarMap.count("csv") > 0)
			{
				ChordEvaluationStats theStats(theConfusionMatrix);
				if (theDuration > 0.)
				{
					theCSVFile << theCSVQuotes << *i << theCSVQuotes << theCSVSeparator
						<< 100*theChordEvaluation.getOverlapScore() << theCSVSeparator
						<< theDuration << theCSVSeparator
						<< 100*theStats.getCorrectChords()/theDuration << theCSVSeparator
						<< 100*theStats.getOnlyRootCorrect()/theDuration << theCSVSeparator
						<< 100*theStats.getOnlyTypeCorrect()/theDuration << theCSVSeparator
						<< 100*theStats.getAllWrong()/theDuration << theCSVSeparator
						<< theStats.getNumOfUniquesInRef() << theCSVSeparator
						<< theStats.getNumOfUniquesInTest() << endl;
				}
				else
				{
					theCSVFile << theCSVQuotes << *i << theCSVQuotes << theCSVSeparator 
						<< "n/a" << theCSVSeparator
						<< theDuration << theCSVSeparator
						<< "n/a" << theCSVSeparator
						<< "n/a" << theCSVSeparator
						<< "n/a" << theCSVSeparator
						<< "n/a" << theCSVSeparator
						<< theStats.getNumOfUniquesInRef() << theCSVSeparator
						<< theStats.getNumOfUniquesInTest() << endl;
				}
			}
		}
		theCSVFile.close();
	}
	/*********/
	/* Notes */
	/*********/
	else if (theVarMap.count("notes") > 0)
	{
		PairwiseEvaluation<Note> theNoteEvaluation(theVarMap["notes"].as<string>());
		theGlobalConfusionMatrix = Eigen::ArrayXXd::Zero(theNoteEvaluation.getNumOfLabels(),
			theNoteEvaluation.getNumOfLabels());
		theLabels = theNoteEvaluation.getLabels();

		ofstream theCSVFile;
		if (theVarMap.count("csv") > 0)
		{
			theCSVFile.open(path(theOutputPath).replace_extension("csv"));
			theCSVFile << theVarMap["notes"].as<string>() << endl;
			theCSVFile << "File" 
				<< theCSVSeparator << "Pairwise score (%)"
				<< theCSVSeparator << "Duration (s)" 
				<< theCSVSeparator << "Correct notes (%)" 
				<< theCSVSeparator << "Octave error (%)" 
				<< theCSVSeparator << "Fifth error (%)" 
				<< theCSVSeparator << "Chromatic up error (%)"
				<< theCSVSeparator << "Chromatic down error (%)"
				<< theCSVSeparator << "Correct no-notes (%)" 
				<< theCSVSeparator << "Note deletions (%)" 
				<< theCSVSeparator << "Note insertions (%)"  << endl;
			theCSVFile << std::fixed;
		}

		for (vector<string>::const_iterator i = theListItems.begin(); i != theListItems.end(); ++i)
		{
			cout << "Evaluating file " << *i << endl;
            constructPaths(*i, theRefDirName, theRefExt, theTestDirName, theTestExt, theVarMap, theRefPath, theTestPath, theBegin, theEnd);

			NoteFileMaps theTestFile(theTestPath.string());
			NoteSequence theTestNotes = theTestFile.readAll();

			NoteFileMaps theRefFile(theRefPath.string());
			NoteSequence theRefNotes = theRefFile.readAll();

            ofstream theVerboseStream;
            if (theVarMap.count("verbose"))
            {
                path theVerbosePath = theOutputPath.parent_path() / path(*i + ".csv");
                theVerboseStream.open(theVerbosePath);
            }
			theNoteEvaluation.evaluate(theRefNotes, theTestNotes, theBegin, theEnd, theVerboseStream, theDelay);
			double theDuration = theNoteEvaluation.getTotalDuration();
			theTotalDuration += theDuration;
			theWeightedScore += theNoteEvaluation.getOverlapScore() * theDuration;
			Eigen::ArrayXXd theConfusionMatrix = theNoteEvaluation.getConfusionMatrix();
			theGlobalConfusionMatrix += theConfusionMatrix;

			if (theVarMap.count("csv") > 0)
			{
				NoteEvaluationStats theStats(theConfusionMatrix);
				theCSVFile << theCSVQuotes << *i << theCSVQuotes << theCSVSeparator
					<< 100*theNoteEvaluation.getOverlapScore() << theCSVSeparator
					<< theDuration << theCSVSeparator
					<< 100*theStats.getCorrectNotes()/theDuration << theCSVSeparator
					<< 100*theStats.getOctaveErrors()/theDuration << theCSVSeparator
					<< 100*theStats.getFifthErrors()/theDuration << theCSVSeparator
					<< 100*theStats.getChromaticUpErrors()/theDuration << theCSVSeparator
					<< 100*theStats.getChromaticDownErrors()/theDuration
					<< 100*theStats.getCorrectNoNotes()/theDuration << theCSVSeparator
					<< 100*theStats.getNoteDeletions()/theDuration << theCSVSeparator
					<< 100*theStats.getNoteInsertions()/theDuration << theCSVSeparator << endl;
			}
		}
		theCSVFile.close();
	}
	/****************/
	/* Segmentation */
	/****************/
	else if (theVarMap.count("segmentation") > 0)
	{
        std::vector<double> tol(1, 0.5);
        SegmentationEvaluation theSegmentationEvaluation(theVarMap["segmentation"].as<string>(), tol);
        
        ofstream theCSVFile;
		if (theVarMap.count("csv") > 0)
		{
			theCSVFile.open(path(theOutputPath).replace_extension("csv"));
			theCSVFile << theVarMap["segmentation"].as<string>() << endl;
			theCSVFile << "File"
            << theCSVSeparator << "DirectionalHammingMeasure"
            << theCSVSeparator << "UnderSegmentation"
            << theCSVSeparator << "OverSegmentation"
//            << theCSVSeparator << "SegmentationRecall"
//            << theCSVSeparator << "SegmentationPrecision"
            << endl;
			theCSVFile << std::fixed;
		}
        
        double theDirectionalHammingSum = 0.;
        double theUnderSegmentationSum = 0.;
        double theOverSegmentationSum = 0.;
		for (vector<string>::const_iterator i = theListItems.begin(); i != theListItems.end(); ++i)
		{
			cout << "Evaluating file " << *i << endl;
            constructPaths(*i, theRefDirName, theRefExt, theTestDirName, theTestExt, theVarMap, theRefPath, theTestPath, theBegin, theEnd);
            
			LabFile<std::string> theRefFile(theRefPath.string(), true);
            LabFile<std::string> theTestFile(theTestPath.string(), true);
            theSegmentationEvaluation.evaluate(theRefFile.readAll(), theTestFile.readAll());
			theDirectionalHammingSum += theSegmentationEvaluation.getDirectionalHammingMeasure();
            theUnderSegmentationSum += theSegmentationEvaluation.getUnderSegmentation();
            theOverSegmentationSum += theSegmentationEvaluation.getOverSegmentation();
            
			if (theVarMap.count("csv") > 0)
			{
				theCSVFile << theCSVQuotes << *i << theCSVQuotes
                << theCSVSeparator << theSegmentationEvaluation.getDirectionalHammingMeasure()
                << theCSVSeparator << theSegmentationEvaluation.getUnderSegmentation()
                << theCSVSeparator << theSegmentationEvaluation.getOverSegmentation()
                << endl;
			}
		}
		theCSVFile.close();
        
        string theSegmentationMode = theVarMap["segmentation"].as<string>();
		theOutputFile << string(theSegmentationMode.size()+17,'*') << "\n* Segmentation " << theSegmentationMode << " *\n"
        << string(theSegmentationMode.size()+17,'*') << endl;
        theOutputFile << "Directional Hamming measure: " << theDirectionalHammingSum / theListItems.size() << endl;
        theOutputFile << "Under-segmentation: " << theUnderSegmentationSum / theListItems.size() << endl;
        theOutputFile << "Over-segmentation: " << theOverSegmentationSum / theListItems.size() << endl;
    }

    /**********************/
	/* Global output file */
    /**********************/
	if (theVarMap.count("chords") > 0)
	{
		string theChordMode = theVarMap["chords"].as<string>();
		theOutputFile << string(theChordMode.size()+11,'*') << "\n* Chords " << theChordMode << " *\n" 
			<< string(theChordMode.size()+11,'*') << endl;
		theOutputFile << "Duration of evaluated symbols: " << theTotalDuration << " s" << endl;
		theOutputFile << "Average score: " << 100 * theWeightedScore / theTotalDuration << "%\n" << endl;

		ChordEvaluationStats theGlobalStats(theGlobalConfusionMatrix);
		theOutputFile << "Correct chords: " << theGlobalStats.getCorrectChords() << "s (" 
			<< 100 * theGlobalStats.getCorrectChords() / theTotalDuration << "%)"  << endl;
		theOutputFile << "Only root correct: " << theGlobalStats.getOnlyRootCorrect() << "s (" 
			<< 100 * theGlobalStats.getOnlyRootCorrect() / theTotalDuration << "%)"  << endl;
		theOutputFile << "Only type correct: " << theGlobalStats.getOnlyTypeCorrect() << "s (" 
			<< 100 * theGlobalStats.getOnlyTypeCorrect() / theTotalDuration << "%)"  << endl;
		theOutputFile << "All wrong: " << theGlobalStats.getAllWrong() << "s (" 
            << 100 * theGlobalStats.getAllWrong() / theTotalDuration << "%)"  << endl;
		theOutputFile << "Deleted chords: " << theGlobalStats.getChordDeletions() << " s ("
            << 100 * theGlobalStats.getChordDeletions() / theTotalDuration << "%)\n" << endl;

		theOutputFile << "Correct no-chords: " << theGlobalStats.getCorrectNoChords() << " s ("
			<< 100 * theGlobalStats.getCorrectNoChords() / theTotalDuration << "%)" << endl;
		theOutputFile << "Inserted chords: " << theGlobalStats.getChordInsertions() << " s ("
			<< 100 * theGlobalStats.getChordInsertions() / theTotalDuration << "%)" << endl;
	}
	else if (theVarMap.count("keys") > 0 || theVarMap.count("globalkey") > 0)
	{
		if (theVarMap.count("keys") > 0)
		{
			string theKeyMode = theVarMap["keys"].as<string>();
			theOutputFile << string(theKeyMode.size()+9,'*') << "\n* Keys " << theKeyMode << " *\n" 
				<< string(theKeyMode.size()+9,'*') << endl;

			theOutputFile << "Duration of evaluated keys: " << theTotalDuration << " s" << endl;
		}
		else
		{
			string theGlobalKeyMode = theVarMap["globalkey"].as<string>();
			theOutputFile << string(theGlobalKeyMode.size()+15,'*') << "\n* Global key " 
				<< theGlobalKeyMode << " *\n" << string(theGlobalKeyMode.size()+15,'*') << endl;

			theOutputFile << "Number of evaluated files: " << theTotalDuration << endl;
		}

		theOutputFile << "Average score: " << 100 * theWeightedScore / theTotalDuration << "%\n" << endl;

		KeyEvaluationStats theGlobalStats(theGlobalConfusionMatrix);
		theOutputFile << "Correct keys: " << theGlobalStats.getCorrectKeys() << " s ("
			<< 100 * theGlobalStats.getCorrectKeys() / theTotalDuration << "%)" << endl;
		theOutputFile << "Adjacent keys: " << theGlobalStats.getAdjacentKeys() << " s ("
			<< 100 * theGlobalStats.getAdjacentKeys() / theTotalDuration << "%)" << endl;
		theOutputFile << "Relative keys: " << theGlobalStats.getRelativeKeys() << " s ("
			<< 100 * theGlobalStats.getRelativeKeys() / theTotalDuration << "%)" << endl;
		theOutputFile << "Parallel keys: " << theGlobalStats.getParallelKeys() << " s ("
			<< 100 * theGlobalStats.getParallelKeys() / theTotalDuration << "%)" << endl;

		if (theVarMap.count("keys") > 0)
		{
			theOutputFile << "Deleted keys: " << theGlobalStats.getKeyDeletions() << " s ("
                << 100 * theGlobalStats.getKeyDeletions() / theTotalDuration << "%)\n" << endl;
			theOutputFile << "Correct no-keys: " << theGlobalStats.getCorrectNoKeys() << " s ("
                << 100 * theGlobalStats.getCorrectNoKeys() / theTotalDuration << "%)" << endl;
			theOutputFile << "Inserted keys: " << theGlobalStats.getKeyInsertions() << " s ("
                << 100 * theGlobalStats.getKeyInsertions() / theTotalDuration << "%)" << endl;
		}
	}
	else if (theVarMap.count("notes") > 0)
	{
		string theNoteMode = theVarMap["notes"].as<string>();
		theOutputFile << string(theNoteMode.size()+10,'*') << "\n* Notes " << theNoteMode << " *\n" 
			<< string(theNoteMode.size()+10,'*') << endl;
		theOutputFile << "Duration of evaluated notes: " << theTotalDuration << " s" << endl;
		theOutputFile << "Average score: " << 100 * theWeightedScore / theTotalDuration << "%\n" << endl;

		NoteEvaluationStats theGlobalStats(theGlobalConfusionMatrix);
		theOutputFile << "Correct notes: " << theGlobalStats.getCorrectNotes() << " s ("
			<< 100 * theGlobalStats.getCorrectNotes() / theTotalDuration << "%)\n";
		theOutputFile << "Octave errors: " << theGlobalStats.getOctaveErrors() << " s ("
			<< 100 * theGlobalStats.getOctaveErrors() / theTotalDuration << "%)\n";
		theOutputFile << "Fifth errors: " << theGlobalStats.getFifthErrors() << " s ("
			<< 100 * theGlobalStats.getFifthErrors() / theTotalDuration << "%)\n";
		theOutputFile << "Chromatic up errors: " << theGlobalStats.getChromaticUpErrors() << " s ("
			<< 100 * theGlobalStats.getChromaticUpErrors() / theTotalDuration << "%)\n";
		theOutputFile << "Chromatic down errors: " << theGlobalStats.getChromaticDownErrors() << " s ("
            << 100 * theGlobalStats.getChromaticDownErrors() / theTotalDuration << "%)" << endl;
		theOutputFile << "Deleted notes: " << theGlobalStats.getNoteDeletions() << " s ("
            << 100 * theGlobalStats.getNoteDeletions() / theTotalDuration << "%)\n" << endl;

		theOutputFile << "Correct no-notes: " << theGlobalStats.getCorrectNoNotes() << " s ("
			<< 100 * theGlobalStats.getCorrectNoNotes() / theTotalDuration << "%)" << endl;
		theOutputFile << "Inserted notes: " << theGlobalStats.getNoteInsertions() << " s ("
			<< 100 * theGlobalStats.getNoteInsertions() / theTotalDuration << "%)" << endl;
	}

    /********************/
	/* Confusion matrix */
    /********************/
	if (theVarMap.count("confusion") && theVarMap.count("segmentation") == 0)
	{
		ofstream theConfusionMatrixFile(theVarMap["confusion"].as<path>());
		if (!theConfusionMatrixFile.is_open())
		{
			throw runtime_error("Could not open confusion matrix file " + 
				theVarMap["confusion"].as<path>().string());
		}
		printConfusionMatrix(theConfusionMatrixFile, theGlobalConfusionMatrix, theLabels);
	}

	cout << "Total execution time: " << static_cast<double>(clock())/CLOCKS_PER_SEC << "s" << endl;
	return 0;
}
catch (exception& e)
{
	cerr << "Error: " << e.what() << endl;
	return -1;
} }
