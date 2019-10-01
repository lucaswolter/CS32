#include "provided.h"
#include "Trie.h"
#include <algorithm>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
using namespace std;

class GenomeMatcherImpl
{
public:
	GenomeMatcherImpl(int minSearchLength);
	void addGenome(const Genome& genome);
	int minimumSearchLength() const;
	bool findGenomesWithThisDNA(const string& fragment, int minimumLength, bool exactMatchOnly, vector<DNAMatch>& matches) const;
	bool findRelatedGenomes(const Genome& query, int fragmentMatchLength, bool exactMatchOnly, double matchPercentThreshold, vector<GenomeMatch>& results) const;
private:
	struct SeqFrag;
	int m_minSearchLength;
	vector<Genome> m_genomeList;
	Trie<SeqFrag> m_seqFragTrie;

		// called by findGenomesWithThisDNA
	DNAMatch findMatch(const string &fragment, const SeqFrag &match, bool exactMatchOnly) const;
	bool sameGenome(const DNAMatch &newMatch, const vector<DNAMatch> &existingMatches, int &genomeInd) const;

		// called by findRelatedGenomes
	int numMatches(const vector<DNAMatch> &allMatches, const string &genomeName) const;
	void insertMatch(const GenomeMatch &match, vector<GenomeMatch> &allMatches) const;
};

//=================================================================================================
//	struct SeqFrag
//	holds the index of the genome from which the sequence fragment comes in m_genomeList and the
//	sequence fragment's position within the genome
//=================================================================================================
struct GenomeMatcherImpl::SeqFrag {
	int genomeIndex;
	int position;
};

//=================================================================================================
//	PUBLIC MEMBERS
//=================================================================================================

//=================================================================================================
//	constructor
//	sets m_minSearchLength to minSearchLength
//=================================================================================================
GenomeMatcherImpl::GenomeMatcherImpl(int minSearchLength)
	: m_minSearchLength(minSearchLength) {}

//=================================================================================================
//	void addGenome
//	adds genome to m_genomeList and each substring of its DNA sequence of length m_minSearchLength
//	to m_seqFragTrie
//=================================================================================================
void GenomeMatcherImpl::addGenome(const Genome& genome)
{
	m_genomeList.push_back(genome);
	for (size_t i = 0; i <= genome.length() - m_minSearchLength; i++) {
		SeqFrag sf;
		sf.genomeIndex = m_genomeList.size() - 1;
		sf.position = i;
		string fragment;
		genome.extract(i, m_minSearchLength, fragment);
		m_seqFragTrie.insert(fragment, sf);
	}
}

//=================================================================================================
//	int minimumSearchLength
//	returns m_minSearchLength
//=================================================================================================
int GenomeMatcherImpl::minimumSearchLength() const
{
	return m_minSearchLength;
}

//=================================================================================================
//	bool findGenomesWithThisDNA
//	adds any portions of DNA that match fragment up to at least minimumLength to matches and
//	returns true. if no matches found or invalid parameters, returns false.
//=================================================================================================
bool GenomeMatcherImpl::findGenomesWithThisDNA(const string& fragment, int minimumLength, bool exactMatchOnly, vector<DNAMatch>& matches) const
{
	// invalid cases
	if (fragment.size() < minimumLength || minimumLength < m_minSearchLength)
		return false;

	vector<DNAMatch> matchHolder;
	vector<SeqFrag> tempMatches = m_seqFragTrie.find(fragment.substr(0, m_minSearchLength), exactMatchOnly);
	
	// adds any relevant matches to matchHolder
	for (size_t i = 0; i < tempMatches.size(); i++) {
		DNAMatch match = findMatch(fragment, tempMatches[i], exactMatchOnly);
		int repl;
		if (sameGenome(match, matchHolder, repl)) {
			if (match.length > matchHolder[repl].length)
				matchHolder[repl] = match;
		}
		else if(match.length >= minimumLength)
			matchHolder.push_back(match);
	}

	// checks if anything was added to matchHolder and returns
	matches.insert(matches.end(), matchHolder.begin(), matchHolder.end());
	return !matchHolder.empty();
}

//=================================================================================================
//	bool findRelatedGenomes
//	adds any genomes that match query's dna sequence with a percentage greater than
//	matchPercentThreshold and returns true. if no genomes have a large enough match percentage or
//	invalid parameters, return false.
//=================================================================================================
bool GenomeMatcherImpl::findRelatedGenomes(const Genome& query, int fragmentMatchLength, bool exactMatchOnly, double matchPercentThreshold, vector<GenomeMatch>& results) const
{
	// invalid case
	if (fragmentMatchLength < m_minSearchLength)
		return false;

	int numFrags = query.length() / fragmentMatchLength;
	vector<DNAMatch> matches;
	vector<GenomeMatch> matchHolder;

	//adds all DNA matches to matches
	for (size_t i = 0; i < numFrags; i++) {
		string fragment;
		query.extract(i*fragmentMatchLength, fragmentMatchLength, fragment);
		findGenomesWithThisDNA(fragment, fragmentMatchLength, exactMatchOnly, matches);
	}

	// determines match percentage and adds matches over matchPercentThreshold to matchHolder
	for (size_t i = 0; i < m_genomeList.size(); i++) {
		double percentage = (double)numMatches(matches, m_genomeList[i].name()) / numFrags * 100;
		if (percentage > matchPercentThreshold) {
			GenomeMatch gm;
			gm.genomeName = m_genomeList[i].name();
			gm.percentMatch = percentage;
			insertMatch(gm, matchHolder);
		}
	}

	// check if anything was added to matchHolder and returns
	results.insert(results.end(), matchHolder.begin(), matchHolder.end());
	return !matchHolder.empty();
}

//=================================================================================================
//	PRIVATE MEMBERS
//=================================================================================================

//=================================================================================================
//	DNAMatch findMatch
//	finds the length of the given match and returns a DNAMatch object of that match
//=================================================================================================
DNAMatch GenomeMatcherImpl::findMatch(const string &fragment, const SeqFrag &match, bool exactMatchOnly) const {
	// determine the fragment of the genome that should be checked
	int glength = min((int)fragment.size(), m_genomeList[match.genomeIndex].length() - match.position);
	string gfrag;
	m_genomeList[match.genomeIndex].extract(match.position, glength, gfrag);
	int length = 0;

	// determine the length of the match
	for (size_t i = 0; i < gfrag.size(); i++) {
		if (!exactMatchOnly && fragment[i] != gfrag[i])
			exactMatchOnly = true;
		else if (fragment[i] != gfrag[i])
			break;
		length++;
	}

	// create the DNAMatch object
	DNAMatch m;
	m.genomeName = m_genomeList[match.genomeIndex].name();
	m.length = length;
	m.position = match.position;
	return m;
}

//=================================================================================================
//	bool sameGenome
//	returns true if existingMatches already contains a DNAMatch with the same name as newMatch and
//	sets genomeInd to its index. Otherwise, returns false and leaves genomeInd unchanged.
//=================================================================================================
bool GenomeMatcherImpl::sameGenome(const DNAMatch &newMatch, const vector<DNAMatch> &existingMatches, int &genomeInd) const {
	for (size_t i = 0; i < existingMatches.size(); i++) {
		if (newMatch.genomeName == existingMatches[i].genomeName) {
			genomeInd = i;
			return true;
		}
	}
	return false;
}

//=================================================================================================
//	int numMatches
//	returns the number of matches in allMatches that have the name genomeName
//=================================================================================================
int GenomeMatcherImpl::numMatches(const vector<DNAMatch> &allMatches, const string &genomeName) const {
	int count = 0;
	for (size_t i = 0; i < allMatches.size(); i++) {
		if (allMatches[i].genomeName == genomeName)
			count++;
	}
	return count;
}

//=================================================================================================
//	void insertMatch
//	inserts match into allMatches to maintain descending order of percentMatches
//=================================================================================================
void GenomeMatcherImpl::insertMatch(const GenomeMatch &match, vector<GenomeMatch> &allMatches) const {
	vector<GenomeMatch>::iterator it;
	for (it = allMatches.begin(); it != allMatches.end(); it++) {
		if (match.percentMatch > it->percentMatch || match.percentMatch == it->percentMatch && match.genomeName < it->genomeName)
			break;
	}
	allMatches.insert(it, match);
}


//******************** GenomeMatcher functions ********************************

// These functions simply delegate to GenomeMatcherImpl's functions.
// You probably don't want to change any of this code.

GenomeMatcher::GenomeMatcher(int minSearchLength)
{
	m_impl = new GenomeMatcherImpl(minSearchLength);
}

GenomeMatcher::~GenomeMatcher()
{
	delete m_impl;
}

void GenomeMatcher::addGenome(const Genome& genome)
{
	m_impl->addGenome(genome);
}

int GenomeMatcher::minimumSearchLength() const
{
	return m_impl->minimumSearchLength();
}

bool GenomeMatcher::findGenomesWithThisDNA(const string& fragment, int minimumLength, bool exactMatchOnly, vector<DNAMatch>& matches) const
{
	return m_impl->findGenomesWithThisDNA(fragment, minimumLength, exactMatchOnly, matches);
}

bool GenomeMatcher::findRelatedGenomes(const Genome& query, int fragmentMatchLength, bool exactMatchOnly, double matchPercentThreshold, vector<GenomeMatch>& results) const
{
	return m_impl->findRelatedGenomes(query, fragmentMatchLength, exactMatchOnly, matchPercentThreshold, results);
}
