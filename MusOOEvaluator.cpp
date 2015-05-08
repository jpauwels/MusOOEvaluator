//============================================================================
/**	@file

	.
	
	@author		Johan Pauwels
	@date		20100913
*/
//============================================================================

#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <ctime>
#include <algorithm>
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
                      path& outRefPath, path& outTestPath, string& outRefExt, string& outTestExt, string& outRefFormat,
                      string& outTestFormat, double& outBegin, double& outEnd, double& outMinRefDuration, double& outMaxRefDuration, double& outTimeDelay,
                      variables_map& outVarMap)
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
        ("refformat", value<string>(&outRefFormat)->default_value("auto"), "format of the reference file(s)")
        ("testformat", value<string>(&outTestFormat)->default_value("auto"), "format of the file(s) under test")
		("csv", "Print results for individual files to file in comma separated format")
        ("confusion", value<path>(), "path to resulting global confusion matrix")
        ("verbose", "Write comparison file for each individual file")
        ("begin", value<double>(&outBegin)->default_value(0.), "the start time in seconds")
        ("end", value<double>(&outEnd)->default_value(0., "file end"), "the end time in seconds")
        ("minduration", value<double>(&outMinRefDuration)->default_value(0.), "minimum duration the reference label needs to have to be included in evaluation")
        ("maxduration", value<double>(&outMaxRefDuration)->default_value(std::numeric_limits<double>::infinity(), "inf"), "maximum duration the reference label is allowed to have to be included in evaluation")
        ("delay", value<double>(&outTimeDelay)->default_value(0.), "Add a time delay to the files to evaluate")
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
            throw invalid_argument("'" + outListPath.string() + "' is not an existing file");
        }
        if (outVarMap.count("refdir") > 0)
        {
             if(!is_directory(outRefPath))
            {
                throw invalid_argument("'" + outRefPath.string() + "' is not an existing directory");
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
                throw invalid_argument("'" + outTestPath.string() + "' is not an existing directory");
            }
        }
        else
        {
            throw invalid_argument("Please specifiy a test directory");
        }
        if (outVarMap.count("timingdir") > 0 && !is_directory(outVarMap["timingdir"].as<path>()))
        {
            throw invalid_argument("'" + outVarMap["timingdir"].as<path>().string() + "' is not an existing directory");
        }
    }
    else
    {
        // Single file mode
        if (outVarMap.count("reffile") > 0)
        {
            if (!is_regular_file(outRefPath))
            {
                throw invalid_argument("'" + outRefPath.string() + "' is not an existing file");
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
                throw invalid_argument("'" + outTestPath.string() + "' is not an existing file");
            }
        }
        else
        {
            throw invalid_argument("Please specifiy a test file");
        }
        if (outVarMap.count("timingfile") > 0 && !is_regular_file(outVarMap["timingfile"].as<path>()))
        {
            throw invalid_argument("'" + outVarMap["timingfile"].as<path>().string() + "' is not an existing file");
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
            throw runtime_error("Could not open list '" + inListPath.string() + "'");
        }
        std::stringstream theFileStream;
        normaliseLineEndings(theListIn, theFileStream);
        //run over all files in the list
        string theListItem;
        while(getline(theFileStream, theListItem))
        {
            theListItems.push_back(theListItem);
        }
        return theListItems;
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

void constructPaths(std::string& ioBaseName, const path& theRefDirName, const std::string& theRefExt, const path& theTestDirName, const std::string& theTestExt, const variables_map& theVarMap, path& outRefFileName, path& outTestFileName, double& outBegin, double& outEnd)
{
    if (ioBaseName.empty())
    {
        ioBaseName = theTestDirName.stem().string();
        outRefFileName = theRefDirName;
        outTestFileName = theTestDirName;
    }
    else
    {
        outRefFileName = theRefDirName / path(ioBaseName + theRefExt);
        outTestFileName = theTestDirName / path(ioBaseName + theTestExt);
    }
    if (theVarMap.count("timingdir") > 0 || theVarMap.count("timingfile") > 0)
    {
        path theTimingPath;
        if (theVarMap.count("timingdir") > 0)
        {
            theTimingPath = theVarMap["timingdir"].as<path>() / path(ioBaseName + theVarMap["timingext"].as<string>());
        }
        else
        {
            theTimingPath = theVarMap["timingfile"].as<path>();
        }
        boost::filesystem::ifstream theTimingFile(theTimingPath);
        if (!theTimingFile.is_open())
        {
            throw runtime_error("Could not open file '" + theTimingPath.string() + "' for reading.");
        }
        theTimingFile >> outBegin >> outEnd;
    }
}

const std::string printResultLine(const double inResult, const double inTotal, const std::string& inUnit)
{
    std::ostringstream stringStream;
    stringStream << inResult << inUnit << " (";
    if (inTotal > 0)
    {
        stringStream << 100 * inResult / inTotal << "%)";
    }
    else
    {
        stringStream << "n/a)";
    }
    return stringStream.str();
}

/**	Start point of the program. */
int main(int inNumOfArguments,char* inArguments[])
{ try {

	path theOutputPath;
	path theListPath;
	path theRefDirPath;
	path theTestDirPath;
	string theRefExt;
	string theTestExt;
    string theRefFormat;
    string theTestFormat;
	double theBegin;
	double theEnd;
    double theMinRefDuration;
    double theMaxRefDuration;
	double theDelay;
	variables_map theVarMap;

	parseCommandLine(inNumOfArguments, inArguments, theOutputPath, theListPath,
		theRefDirPath, theTestDirPath, theRefExt, theTestExt, theRefFormat, theTestFormat,
		theBegin, theEnd, theMinRefDuration, theMaxRefDuration, theDelay, theVarMap);

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
		throw runtime_error("Could not open output file '" + theOutputPath.string() + "'");
	}
    if (!theListPath.empty())
    {
        theOutputFile << "List: " << theListPath << "\n" << endl;
    }
    else
    {
        theOutputFile << "Files: " << theRefDirPath << " vs " << theTestDirPath << "\n" << endl;
    }

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
			Eigen::ArrayXXd::Zero(theKeyEvaluation->getNumOfTestLabels(),
			theKeyEvaluation->getNumOfRefLabels());
        theLabels.resize(theKeyEvaluation->getNumOfTestLabels());
        std::transform(theKeyEvaluation->getLabels().begin(), theKeyEvaluation->getLabels().end(), theLabels.begin(), std::mem_fun_ref(&MusOO::KeyQM::str));

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

		for (vector<string>::iterator i = theListItems.begin(); i != theListItems.end(); ++i)
		{
            constructPaths(*i, theRefDirPath, theRefExt, theTestDirPath, theTestExt, theVarMap, theRefPath, theTestPath, theBegin, theEnd);
			cout << "Evaluating file " << *i << endl;
            
            TimedKeySequence theRefKeys = KeyFileUtil::readKeySequenceFromFile(theRefPath, true, theRefFormat);
			TimedKeySequence theTestKeys = KeyFileUtil::readKeySequenceFromFile(theTestPath, false, theTestFormat);
            
			if (theVarMap.count("keys") > 0)
			{
                ofstream theVerboseStream;
                if (theVarMap.count("verbose"))
                {
                    path theVerbosePath = theOutputPath.parent_path() / path(*i + ".csv");
                    theVerboseStream.open(theVerbosePath);
                }
				theKeyEvaluation->evaluate(theRefKeys, theTestKeys, theBegin, theEnd, theVerboseStream, theMinRefDuration, theMaxRefDuration, theDelay);
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
        
        // Global output file
        string theUnit;
		if (theVarMap.count("keys") > 0)
		{
            theUnit = " s";
			string theKeyMode = theVarMap["keys"].as<string>();
			theOutputFile << string(theKeyMode.size()+9,'*') << "\n* Keys " << theKeyMode << " *\n" << string(theKeyMode.size()+9,'*') << endl;
			theOutputFile << "Duration of evaluated keys: " << theTotalDuration << " s" << endl;
		}
		else
		{
			string theGlobalKeyMode = theVarMap["globalkey"].as<string>();
			theOutputFile << string(theGlobalKeyMode.size()+15,'*') << "\n* Global key " << theGlobalKeyMode << " *\n" << string(theGlobalKeyMode.size()+15,'*') << endl;
			theOutputFile << "Number of evaluated files: " << theTotalDuration << endl;
		}
        
		theOutputFile << "Average score: " << 100 * theWeightedScore / theTotalDuration << "%\n" << endl;
        
		KeyEvaluationStats theGlobalStats(theGlobalConfusionMatrix);
        theOutputFile << "Correct keys: " << printResultLine(theGlobalStats.getCorrectKeys(), theTotalDuration, theUnit) << endl;
        theOutputFile << "Adjacent keys: " << printResultLine(theGlobalStats.getAdjacentKeys(), theTotalDuration, theUnit) << endl;
        theOutputFile << "Relative keys: " << printResultLine(theGlobalStats.getRelativeKeys(), theTotalDuration, theUnit) << endl;
        theOutputFile << "Parallel keys: " << printResultLine(theGlobalStats.getParallelKeys(), theTotalDuration, theUnit) << endl;
        
		if (theVarMap.count("keys") > 0)
		{
            theOutputFile << "Deleted keys: " << printResultLine(theGlobalStats.getKeyDeletions(), theTotalDuration, theUnit) << endl;
            theOutputFile << "\nCorrect no-keys: " << printResultLine(theGlobalStats.getCorrectNoKeys(), theTotalDuration, theUnit) << endl;
            theOutputFile << "Inserted keys: " << printResultLine(theGlobalStats.getKeyInsertions(), theTotalDuration, theUnit) << endl;
		}
        
        theOutputFile << "\nResults per mode\n" << "----------------" << endl;
        const Eigen::ArrayXXd theResultsPerMode = theGlobalStats.getCorrectKeysPerMode();
        for (size_t iMode = 0; iMode < theResultsPerMode.rows(); ++iMode)
        {
            theOutputFile << ModeQM(theKeyEvaluation->getLabels()[iMode].mode()) << ": "
                << printResultLine(theResultsPerMode(iMode,0), theResultsPerMode(iMode,1), theUnit) << " of "
                << printResultLine(theResultsPerMode(iMode,1), theTotalDuration, theUnit) << endl;
        }
		delete theKeyEvaluation;
	}
	/**********/
	/* Chords */
	/**********/
	else if (theVarMap.count("chords") > 0)
	{
		PairwiseEvaluation<Chord> theChordEvaluation(theVarMap["chords"].as<string>());
		theGlobalConfusionMatrix = Eigen::ArrayXXd::Zero(theChordEvaluation.getNumOfTestLabels(),
                                                         theChordEvaluation.getNumOfRefLabels());
        theLabels.resize(theChordEvaluation.getNumOfTestLabels());
        std::transform(theChordEvaluation.getLabels().begin(), theChordEvaluation.getLabels().end(), theLabels.begin(), std::mem_fun_ref(&MusOO::ChordQM::str));
        const size_t numChordTypes = theChordEvaluation.getNumOfRefLabels()/12;
        vector<size_t> cardinalities(numChordTypes);
        std::transform(theChordEvaluation.getLabels().begin(), theChordEvaluation.getLabels().begin()+numChordTypes, cardinalities.begin(), std::mem_fun_ref(&MusOO::Chord::cardinality));
        const size_t maxCardinality = *std::max_element(cardinalities.begin(), cardinalities.end());
        const size_t minCardinality = *std::min_element(cardinalities.begin(), cardinalities.end());

		ofstream theCSVFile;
		if (theVarMap.count("csv") > 0)
		{
			theCSVFile.open(path(theOutputPath).replace_extension("csv"));
			theCSVFile << theVarMap["chords"].as<string>() << endl;
			theCSVFile << "File" 
				<< theCSVSeparator << "Pairwise score (%)"
                << theCSVSeparator << "Duration (s)"
                << theCSVSeparator << "Correct chords (%)"
                << theCSVSeparator << "Substituted chords (%)"
                << theCSVSeparator << "Deleted chords (%)"
                << theCSVSeparator << "Inserted chords (%)"
                << theCSVSeparator << "Correct no-chords (%)";
            for (size_t iChordType = 0; iChordType < numChordTypes; ++iChordType)
            {
                theCSVFile << theCSVSeparator << theCSVQuotes << ChordTypeQM(theChordEvaluation.getLabels()[iChordType].type()) << " correct (%)" << theCSVQuotes << theCSVSeparator << theCSVQuotes << ChordTypeQM(theChordEvaluation.getLabels()[iChordType].type()) << " proportion (%)" << theCSVQuotes;
            }
            for (size_t iNumOfWrongChromas = 0; iNumOfWrongChromas <= maxCardinality; ++iNumOfWrongChromas)
            {
                theCSVFile << theCSVSeparator << iNumOfWrongChromas << " chroma" << (iNumOfWrongChromas==1?"":"s") << " wrong (%)";
            }
            theCSVFile
				<< theCSVSeparator << "Both correct (%)"
				<< theCSVSeparator << "Only root correct (%)"
				<< theCSVSeparator << "Only type correct (%)"
				<< theCSVSeparator << "Both wrong (%)"
				<< theCSVSeparator << "Unique ref chords"
				<< theCSVSeparator << "Unique test chords" << endl;
			theCSVFile << std::fixed;
		}

		for (vector<string>::iterator i = theListItems.begin(); i != theListItems.end(); ++i)
		{
            constructPaths(*i, theRefDirPath, theRefExt, theTestDirPath, theTestExt, theVarMap, theRefPath, theTestPath, theBegin, theEnd);
			cout << "Evaluating file " << *i << endl;
            
			TimedChordSequence theRefChords = ChordFileUtil::readChordSequenceFromFile(theRefPath, true, theRefFormat);
            TimedChordSequence theTestChords = ChordFileUtil::readChordSequenceFromFile(theTestPath, false, theTestFormat);

            ofstream theVerboseStream;
            if (theVarMap.count("verbose"))
            {
                path theVerbosePath = theOutputPath.parent_path() / path(*i + ".csv");
                theVerboseStream.open(theVerbosePath);
                theVerboseStream << "Start" << theCSVSeparator << "End" << theCSVSeparator << "RefLabel" << theCSVSeparator << "TestLabel" << theCSVSeparator << "Score" << theCSVSeparator << "Duration" << theCSVSeparator << "MappedRefLabel" << theCSVSeparator << "MappedTestLabel" << theCSVSeparator << "RefChromas" << theCSVSeparator << "TestChromas" << theCSVSeparator << "NumCommonChromas" << theCSVSeparator << "RefBass" << theCSVSeparator << "TestBass" << endl;
            }
			theChordEvaluation.evaluate(theRefChords, theTestChords, theBegin, theEnd, theVerboseStream, theMinRefDuration, theMaxRefDuration, theDelay);
			double theDuration = theChordEvaluation.getTotalDuration();
			theTotalDuration += theDuration;
			theWeightedScore += theChordEvaluation.getOverlapScore() * theDuration;
			Eigen::ArrayXXd theConfusionMatrix = theChordEvaluation.getConfusionMatrix();
			theGlobalConfusionMatrix += theConfusionMatrix;

			
			if (theVarMap.count("csv") > 0)
			{
                theCSVFile << theCSVQuotes << *i << theCSVQuotes << theCSVSeparator;
				ChordEvaluationStats theStats(theConfusionMatrix, theChordEvaluation.getLabels());
				if (theDuration > 0.)
				{
					theCSVFile << 100*theChordEvaluation.getOverlapScore() << theCSVSeparator << theDuration;
                    theCSVFile
                        << theCSVSeparator << 100*theStats.getCorrectChords()/theDuration
                        << theCSVSeparator << 100*theStats.getChordSubstitutions()/theDuration
                        << theCSVSeparator << 100*theStats.getChordDeletions()/theDuration
                        << theCSVSeparator << 100*theStats.getChordInsertions()/theDuration
                        << theCSVSeparator << 100*theStats.getCorrectNoChords()/theDuration;
                    const Eigen::ArrayXXd theResultsPerType = theStats.getCorrectChordsPerType();
                    for (size_t iChordType = 0; iChordType < numChordTypes; ++iChordType)
                    {
                        theCSVFile
                            << theCSVSeparator << 100*theResultsPerType(iChordType,0)/theResultsPerType(iChordType,1)
                            << theCSVSeparator << 100*theResultsPerType(iChordType,1)/theTotalDuration;
                    }
                    for (size_t iNumOfWrongChromas = 0; iNumOfWrongChromas <= maxCardinality; ++iNumOfWrongChromas)
                    {
                        theCSVFile << theCSVSeparator << 100*theStats.getChordsWithNWrong(iNumOfWrongChromas)/theTotalDuration;
                    }
                    theCSVFile
                        << theCSVSeparator << 100*theStats.getCorrectChords()/theDuration
						<< theCSVSeparator << 100*theStats.getOnlyRootCorrect()/theDuration
						<< theCSVSeparator<< 100*theStats.getOnlyTypeCorrect()/theDuration
						<< theCSVSeparator << 100*theStats.getBothRootAndTypeWrong()/theDuration;
				}
				else
				{
					theCSVFile << "n/a" << theCSVSeparator << theDuration;
                    for (size_t i = 0; i < 9+2*numChordTypes+maxCardinality+1; ++i)
                    {
                        theCSVFile << theCSVSeparator << "n/a";
                    }
				}
                theCSVFile << theCSVSeparator << theStats.getNumOfUniquesInRef() << theCSVSeparator << theStats.getNumOfUniquesInTest() << endl;
			}
		}
		theCSVFile.close();
        
        // Global output file
		string theChordMode = theVarMap["chords"].as<string>();
		theOutputFile << string(theChordMode.size()+11,'*') << "\n* Chords " << theChordMode << " *\n"
        << string(theChordMode.size()+11,'*') << endl;
		theOutputFile << "Duration of evaluated chords: " << theTotalDuration << " s" << endl;
		theOutputFile << "Average score: " << 100 * theWeightedScore / theTotalDuration << "%" << endl;
        
		ChordEvaluationStats theGlobalStats(theGlobalConfusionMatrix, theChordEvaluation.getLabels());
        theOutputFile << "\nChord detection results\n" << "-----------------------" << endl;
		theOutputFile << "Correct chords: " << printResultLine(theGlobalStats.getCorrectChords(), theTotalDuration, " s") << endl;
		theOutputFile << "Substituted chords: " << printResultLine(theGlobalStats.getChordSubstitutions(), theTotalDuration, " s") << endl;
		theOutputFile << "Deleted chords: " << printResultLine(theGlobalStats.getChordDeletions(), theTotalDuration, " s") << endl;
		theOutputFile << "Inserted chords: " << printResultLine(theGlobalStats.getChordInsertions(), theTotalDuration, " s") << endl;
		theOutputFile << "Correct no-chords: " << printResultLine(theGlobalStats.getCorrectNoChords(), theTotalDuration, " s") << endl;
        
        theOutputFile << "\nResults per chord type\n" << "----------------------" << endl;
        const Eigen::ArrayXXd theResultsPerType = theGlobalStats.getCorrectChordsPerType();
        for (size_t iChordType = 0; iChordType < numChordTypes; ++iChordType)
        {
            theOutputFile << ChordTypeQM(theChordEvaluation.getLabels()[iChordType].type()) << ": "
                << printResultLine(theResultsPerType(iChordType,0), theResultsPerType(iChordType,1), " s") << " of "
                << printResultLine(theResultsPerType(iChordType,1), theTotalDuration, " s") << endl;
        }
        
        theOutputFile << "\nResults per number of chromas wrong\n" << "-----------------------------------" << endl;
        theOutputFile << "0 chromas wrong: " << printResultLine(theGlobalStats.getChordsWithNWrong(0), theTotalDuration, " s") << endl;
        theOutputFile << "  of which root correct: " << printResultLine(theGlobalStats.getCorrectChords(), theTotalDuration, " s") << endl;
        theOutputFile << "  of which root incorrect: " << printResultLine(theGlobalStats.getChordsWithNWrong(0)-theGlobalStats.getCorrectChords(), theTotalDuration, " s") << endl;
        for (size_t iNumOfWrongChromas = 1; iNumOfWrongChromas <= maxCardinality; ++iNumOfWrongChromas)
        {
            theOutputFile << iNumOfWrongChromas << " chroma" << (iNumOfWrongChromas==1?"":"s") << " wrong: " << printResultLine(theGlobalStats.getChordsWithNWrong(iNumOfWrongChromas), theTotalDuration, " s") << endl;
            theOutputFile << "  of which " << iNumOfWrongChromas << " substitution" << (iNumOfWrongChromas==1?"":"s") << ": " << printResultLine(theGlobalStats.getChordsWithSDI(iNumOfWrongChromas, 0, 0), theTotalDuration, " s") << endl;
            //prune impossible combinations for this set of chord types
            if (minCardinality + iNumOfWrongChromas <= maxCardinality)
            {
                theOutputFile << "  of which " << iNumOfWrongChromas << " deletion" << (iNumOfWrongChromas==1?"":"s") << ": " << printResultLine(theGlobalStats.getChordsWithSDI(0, iNumOfWrongChromas, 0), theTotalDuration, " s") << endl;
            }
            if (maxCardinality - iNumOfWrongChromas >= minCardinality)
            {
                theOutputFile << "  of which " << iNumOfWrongChromas << " insertion" << (iNumOfWrongChromas==1?"":"s") << ": " << printResultLine(theGlobalStats.getChordsWithSDI(0, 0, iNumOfWrongChromas), theTotalDuration, " s") << endl;
            }
            for (size_t iNumSubstitutions = iNumOfWrongChromas-1; iNumSubstitutions > 0; --iNumSubstitutions)
            {
                const size_t numDI = iNumOfWrongChromas-iNumSubstitutions;
                if (minCardinality + numDI <= maxCardinality)
                {
                    theOutputFile << "  of which " << iNumSubstitutions << " substitution" << (iNumSubstitutions==1?"":"s") << ", " << numDI << " deletion" << (numDI==1?"":"s") << ": " << printResultLine(theGlobalStats.getChordsWithSDI(iNumSubstitutions, numDI, 0), theTotalDuration, " s") << endl;
                }
                if (maxCardinality - numDI >= minCardinality)
                {
                    theOutputFile << "  of which " << iNumSubstitutions << " substitution" << (iNumSubstitutions==1?"":"s") << ", " << numDI << " insertion" << (numDI==1?"":"s") << ": " << printResultLine(theGlobalStats.getChordsWithSDI(iNumSubstitutions, 0, numDI), theTotalDuration, " s") << endl;
                }
            }
        }
        const double unknownWrong = theGlobalStats.getChordsWithUnknownWrong();
        if (unknownWrong > 0.)
        {
            theOutputFile << "unknown chromas wrong: " << printResultLine(unknownWrong, theTotalDuration, " s") << endl;
        }
        
        theOutputFile << "\nRoot/type results\n" << "-----------------" << endl;
        theOutputFile << "Both correct: " << printResultLine(theGlobalStats.getCorrectChords(), theTotalDuration, " s") << endl;
		theOutputFile << "Only root correct: " << printResultLine(theGlobalStats.getOnlyRootCorrect(), theTotalDuration, " s") << endl;
		theOutputFile << "Only type correct: " << printResultLine(theGlobalStats.getOnlyTypeCorrect(), theTotalDuration, " s") << endl;
		theOutputFile << "Both wrong: " << printResultLine(theGlobalStats.getBothRootAndTypeWrong(), theTotalDuration, " s") << endl;
	}
	/*********/
	/* Notes */
	/*********/
	else if (theVarMap.count("notes") > 0)
	{
		PairwiseEvaluation<Note> theNoteEvaluation(theVarMap["notes"].as<string>());
		theGlobalConfusionMatrix = Eigen::ArrayXXd::Zero(theNoteEvaluation.getNumOfTestLabels(),
                                                         theNoteEvaluation.getNumOfRefLabels());
        theLabels.resize(theNoteEvaluation.getNumOfTestLabels());
        std::transform(theNoteEvaluation.getLabels().begin(), theNoteEvaluation.getLabels().end(), theLabels.begin(), std::mem_fun_ref(&MusOO::NoteMidi::str));

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

		for (vector<string>::iterator i = theListItems.begin(); i != theListItems.end(); ++i)
		{
            constructPaths(*i, theRefDirPath, theRefExt, theTestDirPath, theTestExt, theVarMap, theRefPath, theTestPath, theBegin, theEnd);
			cout << "Evaluating file " << *i << endl;

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
			theNoteEvaluation.evaluate(theRefNotes, theTestNotes, theBegin, theEnd, theVerboseStream, theMinRefDuration, theMaxRefDuration, theDelay);
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
        
        // Global output file
		string theNoteMode = theVarMap["notes"].as<string>();
		theOutputFile << string(theNoteMode.size()+10,'*') << "\n* Notes " << theNoteMode << " *\n"
        << string(theNoteMode.size()+10,'*') << endl;
		theOutputFile << "Duration of evaluated notes: " << theTotalDuration << " s" << endl;
		theOutputFile << "Average score: " << 100 * theWeightedScore / theTotalDuration << "%\n" << endl;
        
		NoteEvaluationStats theGlobalStats(theGlobalConfusionMatrix);
		theOutputFile << "Correct notes: " << printResultLine(theGlobalStats.getCorrectNotes(), theTotalDuration, " s") << endl;
		theOutputFile << "Octave errors: " << printResultLine(theGlobalStats.getOctaveErrors(), theTotalDuration, " s") << endl;
		theOutputFile << "Fifth errors: " << printResultLine(theGlobalStats.getFifthErrors(), theTotalDuration, " s") << endl;
		theOutputFile << "Chromatic up errors: " << printResultLine(theGlobalStats.getChromaticUpErrors(), theTotalDuration, " s") << endl;
		theOutputFile << "Chromatic down errors: " << printResultLine(theGlobalStats.getChromaticDownErrors(), theTotalDuration, " s") << endl;
		theOutputFile << "Deleted notes: " << printResultLine(theGlobalStats.getNoteDeletions(), theTotalDuration, " s") << endl;
        
		theOutputFile << "\nCorrect no-notes: " << printResultLine(theGlobalStats.getCorrectNoNotes(), theTotalDuration, " s") << endl;
		theOutputFile << "Inserted notes: " << printResultLine(theGlobalStats.getNoteInsertions(), theTotalDuration, " s") << endl;
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
            << theCSVSeparator << "NumRefSegments"
            << theCSVSeparator << "NumTestSegments"
            << theCSVSeparator << "CombinedHammingMeasureMaximum"
            << theCSVSeparator << "CombinedHammingMeasureHarmonic"
            << theCSVSeparator << "UnderSegmentation"
            << theCSVSeparator << "OverSegmentation"
//            << theCSVSeparator << "SegmentationRecall"
//            << theCSVSeparator << "SegmentationPrecision"
            << endl;
			theCSVFile << std::fixed;
		}
        
        double theCombinedHammingMaximumSum = 0.;
        double theCombinedHammingHarmonicSum = 0.;
        double theUnderSegmentationSum = 0.;
        double theOverSegmentationSum = 0.;
		for (vector<string>::iterator i = theListItems.begin(); i != theListItems.end(); ++i)
		{
            constructPaths(*i, theRefDirPath, theRefExt, theTestDirPath, theTestExt, theVarMap, theRefPath, theTestPath, theBegin, theEnd);
			cout << "Evaluating file " << *i << endl;
            
			LabFile<std::string> theRefFile(theRefPath.string(), true);
            LabFile<std::string> theTestFile(theTestPath.string(), true);
            theSegmentationEvaluation.evaluate(theRefFile.readAll(), theTestFile.readAll());
            theCombinedHammingMaximumSum += theSegmentationEvaluation.getCombinedHammingMeasureMaximum();
            theCombinedHammingHarmonicSum += theSegmentationEvaluation.getCombinedHammingMeasureHarmonic();
            theUnderSegmentationSum += theSegmentationEvaluation.getUnderSegmentation();
            theOverSegmentationSum += theSegmentationEvaluation.getOverSegmentation();
            
			if (theVarMap.count("csv") > 0)
			{
                theCSVFile << theCSVQuotes << *i << theCSVQuotes
                << theCSVSeparator << theSegmentationEvaluation.getNumRefSegments()
                << theCSVSeparator << theSegmentationEvaluation.getNumTestSegments()
                << theCSVSeparator << theSegmentationEvaluation.getCombinedHammingMeasureMaximum()
                << theCSVSeparator << theSegmentationEvaluation.getCombinedHammingMeasureHarmonic()
                << theCSVSeparator << theSegmentationEvaluation.getUnderSegmentation()
                << theCSVSeparator << theSegmentationEvaluation.getOverSegmentation()
                << endl;
			}
		}
		theCSVFile.close();
        
        string theSegmentationMode = theVarMap["segmentation"].as<string>();
		theOutputFile << string(theSegmentationMode.size()+17,'*') << "\n* Segmentation " << theSegmentationMode << " *\n"
        << string(theSegmentationMode.size()+17,'*') << endl;
        theOutputFile << "Average number of reference segments: " << theSegmentationEvaluation.calcAverageNumRefSegments() << endl;
        theOutputFile << "Average number of test segments: " << theSegmentationEvaluation.calcAverageNumTestSegments() << endl;
        theOutputFile << "Average combined Hamming measure (maximum): " << theCombinedHammingMaximumSum / theListItems.size() << endl;
        theOutputFile << "Average combined Hamming measure (harmonic): " << theCombinedHammingHarmonicSum / theListItems.size() << endl;
        theOutputFile << "Average under-segmentation: " << theUnderSegmentationSum / theListItems.size() << endl;
        theOutputFile << "Average over-segmentation: " << theOverSegmentationSum / theListItems.size() << endl;
    }

    /********************/
	/* Confusion matrix */
    /********************/
	if (theVarMap.count("confusion") && theVarMap.count("segmentation") == 0)
	{
		ofstream theConfusionMatrixFile(theVarMap["confusion"].as<path>());
		if (!theConfusionMatrixFile.is_open())
		{
			throw runtime_error("Could not open confusion matrix file '" +
				theVarMap["confusion"].as<path>().string() + "'");
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
