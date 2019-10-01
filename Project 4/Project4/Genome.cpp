#include "provided.h"
#include <string>
#include <vector>
#include <iostream>
#include <istream>
#include <cctype>
using namespace std;

class GenomeImpl
{
public:
	GenomeImpl(const string& nm, const string& sequence);
	static bool load(istream& genomeSource, vector<Genome>& genomes);
	int length() const;
	string name() const;
	bool extract(int position, int length, string& fragment) const;
private:
	string m_name;
	string m_sequence;

		// called by load
	static bool isValidName(string &name);
	static bool isValidSequence(string &sequence);
	static bool isValidBase(char &base);
	static void addGenome(vector<Genome> &genomes, string &name, string &sequence);
};

//=================================================================================================
//	PUBLIC MEMBERS
//=================================================================================================

//=================================================================================================
//	constructor
//	initializes m_name and m_sequence to the specified values
//=================================================================================================
GenomeImpl::GenomeImpl(const string& nm, const string& sequence)
	: m_name(nm), m_sequence(sequence) {}

//=================================================================================================
//	bool load
//	loads all genomes from genomeSource and stores them in genomes. Returns true if genomeSource is
//	formatted correctly, false otherwise
//=================================================================================================
bool GenomeImpl::load(istream& genomeSource, vector<Genome>& genomes)
{
	string line, name = "", sequence = "";

	if (!getline(genomeSource, line) || !isValidName(line))	// first line should be a name
		return false;
	name = line;

	while (getline(genomeSource, line)) {
		if (isValidName(line)) {	// line is a name
			if (sequence.empty())	// invalid because new name without any sequence
				return false;
			addGenome(genomes, name, sequence);	// adds last genome and starts new one
			name = line;
		}
		else if (isValidSequence(line)) {	// line is a sequence
			sequence += line;
		}
		else	// line is invalid
			return false;
	}

	if (name.empty() || sequence.empty())	// did not end on a valid genome
		return false;
	addGenome(genomes, name, sequence);	// add final genome
	return true;
}

//=================================================================================================
//	int length
//	returns the length of m_sequence
//=================================================================================================
int GenomeImpl::length() const
{
	return m_sequence.size();
}

//=================================================================================================
//	string name
//	returns m_name
//=================================================================================================
string GenomeImpl::name() const
{
	return m_name;
}

//=================================================================================================
//	bool extract
//	set fragment to the specified substring of m_sequence and return true
//	if unsuccessful, leave fragment unchanged and return false
//=================================================================================================
bool GenomeImpl::extract(int position, int length, string& fragment) const
{
	if (m_sequence.empty())
		return false;
	string temp = "";

	for (int i = position; i < position + length; i++) {
		if (i < 0 || i >= m_sequence.size())
			return false;
		temp += m_sequence[i];
	}

	fragment = temp;
	return true;
}

//=================================================================================================
//	PRIVATE MEMBERS
//=================================================================================================

//=================================================================================================
//	bool isValidName
//	returns true if name is a correctly formatted genome name and removes the '>' from the
//	beginning of name. Otherwise, returns false and leaves name unchanged
//=================================================================================================
bool GenomeImpl::isValidName(string &name) {
	if (name.size() <= 1 || name[0] != '>')
		return false;
	name = name.substr(1);
	return true;
}

//=================================================================================================
//	bool isValidSequence
//	returns true if sequence is a correctly formatted genome sequence and changes all characters to
//	uppercase. Otherwise, returns false and leaves sequence unchanged
//=================================================================================================
bool GenomeImpl::isValidSequence(string &sequence) {
	if (sequence.empty() || sequence.size() > 80)
		return false;
	string temp = sequence;
	for (size_t i = 0; i < temp.size(); i++) {
		if (!isValidBase(temp[i]))
			return false;
	}

	sequence = temp;
	return true;
}

//=================================================================================================
//	bool isValidBase
//	returns true if base is an upper or lowercase  'A', 'C', 'T', 'G', or 'N' and changes base to
//	be uppercase. Otherwise, returns false and leaves base unchanged
//=================================================================================================
bool GenomeImpl::isValidBase(char &base) {
	if (base == 'a' || base == 'c' || base == 't' || base == 'g' || base == 'n')
		base = toupper(base);
	return base == 'A' || base == 'C' || base == 'T' || base == 'G' || base == 'N';
}

//=================================================================================================
//	void addGenome
//	creates a new genome with specified name and sequence and adds it to genomes. Sets name and
//	sequence to empty strings
//=================================================================================================
void GenomeImpl::addGenome(vector<Genome> &genomes, string &name, string &sequence) {
	genomes.push_back(Genome(name, sequence));
	name = "";
	sequence = "";
}

//******************** Genome functions ************************************

// These functions simply delegate to GenomeImpl's functions.
// You probably don't want to change any of this code.

Genome::Genome(const string& nm, const string& sequence)
{
	m_impl = new GenomeImpl(nm, sequence);
}

Genome::~Genome()
{
	delete m_impl;
}

Genome::Genome(const Genome& other)
{
	m_impl = new GenomeImpl(*other.m_impl);
}

Genome& Genome::operator=(const Genome& rhs)
{
	GenomeImpl* newImpl = new GenomeImpl(*rhs.m_impl);
	delete m_impl;
	m_impl = newImpl;
	return *this;
}

bool Genome::load(istream& genomeSource, vector<Genome>& genomes)
{
	return GenomeImpl::load(genomeSource, genomes);
}

int Genome::length() const
{
	return m_impl->length();
}

string Genome::name() const
{
	return m_impl->name();
}

bool Genome::extract(int position, int length, string& fragment) const
{
	return m_impl->extract(position, length, fragment);
}
