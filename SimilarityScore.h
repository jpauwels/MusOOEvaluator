#ifndef SimilarityScore_h
#define SimilarityScore_h

//============================================================================
/**
	Abstract base template class representing a numerical expression for the 
	similarity between two keys or chords.

	@author		Johan Pauwels
	@date		20100914
*/
//============================================================================
#include <string>
#include <vector>
#include "MusOO/Chroma.h"

template <typename T>
class SimilarityScore
{
public:

	/** Default constructor. */
	SimilarityScore();

	/** Destructor. */
	virtual ~SimilarityScore();

	virtual const double score(const T& inRef, const T& inTest) = 0;

	const size_t& getRefIndex() const;
	const size_t& getTestIndex() const;
	const std::vector<T>& getLabels() const;
	const size_t getNumOfRefLabels() const;
	const size_t getNumOfTestLabels() const;
    
    const T& getMappedRefLabel() const;
    const T& getMappedTestLabel() const;

protected:

	static const MusOO::Chroma s_firstChroma;
 	size_t m_NumOfRefLabels;
 	size_t m_NumOfTestLabels;
	size_t m_RefIndex;
	size_t m_TestIndex;
    T m_MappedRefLabel;
    T m_MappedTestLabel;
	std::vector<T> m_Labels;

private:


};

template <typename T>
const MusOO::Chroma SimilarityScore<T>::s_firstChroma(MusOO::Chroma::A());

template <typename T>
SimilarityScore<T>::SimilarityScore()
{
}

template <typename T>
SimilarityScore<T>::~SimilarityScore()
{
}

template <typename T>
const size_t& SimilarityScore<T>::getRefIndex() const
{
	return m_RefIndex;
}

template <typename T>
const size_t& SimilarityScore<T>::getTestIndex() const
{
	return m_TestIndex;
}

template <typename T>
const std::vector<T>& SimilarityScore<T>::getLabels() const
{
	return m_Labels;
}

template <typename T>
const size_t SimilarityScore<T>::getNumOfRefLabels() const
{
	return m_NumOfRefLabels;
}

template <typename T>
const size_t SimilarityScore<T>::getNumOfTestLabels() const
{
	return m_NumOfTestLabels;
}

template <typename T>
const T& SimilarityScore<T>::getMappedRefLabel() const
{
    return m_MappedRefLabel;
}

template <typename T>
const T& SimilarityScore<T>::getMappedTestLabel() const
{
    return m_MappedTestLabel;
}

#endif	// #ifndef SimilarityScore_h
