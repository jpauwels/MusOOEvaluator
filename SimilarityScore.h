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

	const size_t& getRefCategory() const;
	const size_t& getTestCategory() const;
	const std::vector<std::string>& getCategoryLabels() const;
	const size_t getNumOfCategories() const;
    
    const T& getMappedRefLabel() const;
    const T& getMappedTestLabel() const;

protected:

	static const MusOO::Chroma s_firstChroma;
 	size_t m_NumOfCategories;
	size_t m_RefCategory;
	size_t m_TestCategory;
    T m_MappedRefLabel;
    T m_MappedTestLabel;
	std::vector<std::string> m_CategoryLabels;

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
const size_t& SimilarityScore<T>::getRefCategory() const
{
	return m_RefCategory;
}

template <typename T>
const size_t& SimilarityScore<T>::getTestCategory() const
{
	return m_TestCategory;
}

template <typename T>
const std::vector<std::string>& SimilarityScore<T>::getCategoryLabels() const
{
	return m_CategoryLabels;
}

template <typename T>
const size_t SimilarityScore<T>::getNumOfCategories() const
{
	return m_NumOfCategories;
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
