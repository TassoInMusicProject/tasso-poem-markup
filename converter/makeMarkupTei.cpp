//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Oct 29 16:00:53 PDT 2023
// Last Modified: Tue Oct 31 21:25:07 PDT 2023
// Filename:      makeMarkupTei.h
// URL:
// Syntax:        C++14; pugixml
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Convert basic markup encoding of Tasso
//                poetry into TEI version.
//
// Notes:         Enjambments converted from strong/weak to yes
//                https://tei-c.org/release/doc/tei-p5-doc/de/html/ref-att.enjamb.html
//

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>

#include "pugiconfig.hpp"
#include "pugixml.hpp"

#include "HumRegex.h"

using namespace std;
using namespace hum;
using namespace pugi;


class Syllable {
	public:
		std::string text;
		std::string accent;
};


class Line {
	public:
		std::string input;
		std::vector<Syllable> syllables;
		std::string rest;
		std::string rhyme;
		int enjambment = 0; // 1 = weak, 2 = strong
};


class Stanza {
	public:
		std::vector<Line> lines;
};


class Poem {
	public:
		std::vector<Stanza> stanzas;
		std::string catalognum;
		std::string title;
};


bool   readPoemData            (Poem& poem, std::istream& input);
string getRhyme                (const string& info);
string makeLineMet             (vector<Syllable>& syllables);
bool   convertPoemToTei        (xml_document& tei, Poem& poem);
void   printTeiDocument        (xml_document& tei, std::ostream& out);
void   fillTeiHeaderContents   (xml_node& header, Poem& poem);
void   fillTeiBodyContents     (xml_node& body, Poem& poem);
void   addStanzaToTeiBody      (xml_node& body, Stanza& stanza);
void   addLineToLineGroup      (xml_node& linegroup, Line& verseline);
void   addSyllableToLine       (xml_node& line, Syllable& syllable);
void   parseElisionsInSyllable (xml_node seg, const string& text);


/////////////////////////////////////////////////////////

int main(int argc, char** argv) {
	Poem poem;
	readPoemData(poem, cin);
	xml_document tei;
	int status = convertPoemToTei(tei, poem);
	if (status) {
		printTeiDocument(tei, cout);
	}
	return 0;
}

/////////////////////////////////////////////////////////


//////////////////////////////
//
// readPoemData -- Convert string into basic data
//    Returns true if successful converting the poem; otherwise, returns false if error.
//

bool readPoemData(Poem& poem, istream& input) {
	// Load input into an vector of strings:
	poem.stanzas.reserve(20);
	string line;
	bool inStanza = false;
	HumRegex hre;
	while (getline(input, line)) {
		if (hre.search(line, "^@CATALOGNUM\\s*:\\s*([^\\s]+)")) {
			poem.catalognum = hre.getMatch(1);
			continue;
		}
		if (hre.search(line, "^@TITLE\\s*:\\s*(.+)\\s*")) {
			poem.title = hre.getMatch(1);
			continue;
		}
		if (line.compare(0, 8, "@MVERSE:") == 0) {
			poem.stanzas.resize(poem.stanzas.size() + 1);
			inStanza = true;
			// Currently assuming that @MVERSE: has no content after it on the line.
			continue;
		} else if (line.compare(0, 1, "@") == 0) {
			inStanza = false;
		}
		if (!inStanza) {
			continue;
		}
		if (line.find("=") == string::npos) {
			continue;
		}
		string original = line;
		hre.replaceDestructive(original, "+", "--", "g");
		hre.replaceDestructive(line, "", "^\\s+");
		hre.replaceDestructive(line, "", "\\s+$");
		hre.replaceDestructive(line, "+", "--", "g");
		string rest;
		if (hre.search(line, "^\\s*(.*?)(\\s*)(=.*)\\s*$")) {
			rest = hre.getMatch(3);
			line = hre.getMatch(1);
		} else {
			cerr << "GOT HERE FOR STRANGE REASON" << endl;
		}
		hre.replaceDestructive(line, " ", "\\s+", "g");
		vector<string> syllables ;
		hre.split(syllables, line, "\\s+");
		poem.stanzas.back().lines.resize(poem.stanzas.back().lines.size() + 1);
		Line& curline = poem.stanzas.back().lines.back();
		curline.input = original;
		curline.rest = rest;
		curline.rhyme = getRhyme(rest);
		curline.syllables.resize(1);
		for (int i=0; i<(int)line.size(); i++) {
			if ((i > 0) && (line[i] == ' ') && (line[i-1] == '+')) {
				curline.syllables.back().text += " ";
				continue;
			}
			if ((i > 0) && (line[i] == ' ') && (line[i-1] == '-')) {
				// Should not happen, but could be a data-encoding error.
				curline.syllables.back().text += " ";
				continue;
			}
			if ((line[i] == '-') || (line[i] == '+') || (line[i] == ' ')) {
				string value = "";
				value.push_back(line[i]);
				if (value == " ") {
					value = "-";
				}

				curline.syllables.back().accent = value;
				curline.syllables.resize(curline.syllables.size() + 1);
				if ((line[i] == '-') || (line[i] == '+')) {
					continue;
				}
				if (line[i] == ' ') {
					curline.syllables.back().text += " ";
					continue;
				}
			}
			curline.syllables.back().text += line[i];
		}
		// Space is added to start of next syllable.  Move it to the end of the previous syllable.
		for (int i=1; i<(int)curline.syllables.size(); i++) {
			if (curline.syllables.at(i).text.empty()) {
				continue;
			}
			if (curline.syllables.at(i).text.at(0) == ' ') {
				curline.syllables.at(i-1).text += " ";
				curline.syllables.at(i).text = curline.syllables.at(i).text.substr(1);
			}
		}
		// Delete the last syllable if it is empty:
		if (curline.syllables.back().text.empty()) {
			curline.syllables.resize(curline.syllables.size() - 1);
		}
		// Remove space from end of list syllable
		if (!curline.syllables.empty() && (curline.syllables.back().text.back() == ' ')) {
			curline.syllables.back().text.resize(curline.syllables.back().text.size() - 1);
		}
	}

	return true;
}



//////////////////////////////
//
// getRhyme --
//

string getRhyme(const string& info) {
	HumRegex hre;
	if (hre.search(info, "=\\s*([A-Za-z])")) {
		return hre.getMatch(1);
	} else {
		return "";
	}
}



//////////////////////////////
//
// convertPoemToTei -- convert basic format into TEI (via pugixml document).
//    Returns true if successful converting the poem; otherwise, returns false if error.
//

bool convertPoemToTei(xml_document& doc, Poem& poem) {

	// Add XML prolog lines:
	// <?xml version="1.0" encoding="UTF-8"?>
	// <?xml-model href="http://www.tei-c.org/release/xml/tei/custom/schema/relaxng/tei_all.rng" type="application/xml" schematypens="http://relaxng.org/ns/structure/1.0"?>
	// <?xml-model href="http://www.tei-c.org/release/xml/tei/custom/schema/relaxng/tei_all.rng" type="application/xml" schematypens="http://purl.oclc.org/dsdl/schematron"?>

	xml_node declaration = doc.prepend_child(node_declaration);
	declaration.append_attribute("version") = "1.0";
	declaration.append_attribute("encoding") = "UTF-8";

    // Add processing instructions
	xml_node pi1 = doc.append_child(node_pi);
	pi1.set_name("xml-model");
	pi1.set_value("href=\"http://www.tei-c.org/release/xml/tei/custom/schema/relaxng/tei_all.rng\" type=\"application/xml\" schematypens=\"http://relaxng.org/ns/structure/1.0\"");
	xml_node pi2 = doc.append_child(node_pi);
	pi2.set_name("xml-model");
	pi2.set_value("href=\"http://www.tei-c.org/release/xml/tei/custom/schema/relaxng/tei_all.rng\" type=\"application/xml\" schematypens=\"http://purl.oclc.org/dsdl/schematron\"");

	// Add root node:
	xml_node tei = doc.append_child("TEI");
	tei.append_attribute("xmlns") = "http://www.tei-c.org/ns/1.0";

	// Add header (<teiHeader>) and text/body nodes:
	xml_node header = tei.append_child("teiHeader");
	xml_node text = tei.append_child("text");
	xml_node body = text.append_child("body");

	fillTeiHeaderContents(header, poem);
	fillTeiBodyContents(body, poem);

	return true;
}



//////////////////////////////
//
// fillTeiHeaderContents --
// Example:
//    <fileDesc>
//       <titleStmt>
//          <title>Tasso in Music Project: verse anallytical encoding sample</title>
//       </titleStmt>
//       <publicationStmt>
//          <publisher>Tasso in Music Project</publisher>
//          <idno>Trm0048</idno>
//       </publicationStmt>
//       <sourceDesc>
//          <p>Born digital with examples from Tasso in Music</p>
//       </sourceDesc>
//    </fileDesc>
//   <encodingDesc>
//      <metDecl type="met">
//         <metSym value="+">stressed syllable</metSym>
//         <metSym value="-">unstressed syllable</metSym>
//         <metSym value="_">elision (synalepha?)</metSym>
//      </metDecl>
//   </encodingDesc>
//

void fillTeiHeaderContents(xml_node& header, Poem& poem) {
	xml_node fileDesc = header.append_child("fileDesc");
	xml_node encodingDesc = header.append_child("encodingDesc");

	// Fill <fileDesc>:
	xml_node titleStmt = fileDesc.append_child("titleStmt");
	xml_node title = titleStmt.append_child("title");
	title.text().set(poem.title.c_str());

	xml_node publicationStmt = fileDesc.append_child("publicationStmt");
	xml_node publisher = publicationStmt.append_child("publisher");
	publisher.text().set("Tasso in Music Project");

	xml_node idno = publicationStmt.append_child("idno");
	idno.text().set(poem.catalognum.c_str());

	xml_node sourceDesc = fileDesc.append_child("sourceDesc");
	xml_node p2 = sourceDesc.append_child("p");
	p2.text().set("Born digital with examples from Tasso in Music Project");

	// Fill <encodingDesc>:
	xml_node metDecl = encodingDesc.append_child("metDecl");
	metDecl.append_attribute("type") = "met";

	xml_node stressed = metDecl.append_child("metSym");
	stressed.append_attribute("value") = "+";
	stressed.text().set("stressed syllable");

	xml_node unstressed = metDecl.append_child("metSym");
	unstressed.append_attribute("value") = "-";
	unstressed.text().set("unstressed syllable");

	xml_node elision = metDecl.append_child("metSym");
	elision.append_attribute("value") = "_";
	elision.text().set("elision (synalepha?)");
}



//////////////////////////////
//
// fillTeiBodyContents --
//   <lg> == stanzas
//     <l rhyme n enjamb>
//        <seg>
//

void fillTeiBodyContents(xml_node& body, Poem& poem) {
	for (int i=0; i<(int)poem.stanzas.size(); i++) {
		addStanzaToTeiBody(body, poem.stanzas.at(i));
	}
}



//////////////////////////////
//
// AddStanzaToTeiBody --
//

void addStanzaToTeiBody(xml_node& body, Stanza& stanza) {
	xml_node linegroup = body.append_child("lg");
	for (int i=0; i<(int)stanza.lines.size(); i++) {
		addLineToLineGroup(linegroup, stanza.lines.at(i));
	}
}



//////////////////////////////
//
// addLineToLineGroup --
//

void addLineToLineGroup(xml_node& linegroup, Line& line) {
	xml_node l = linegroup.append_child("l");

	// Add rhyme scheme:
	if (!line.rhyme.empty()) {
		string rhyme = line.rhyme;
		std::transform(rhyme.begin(), rhyme.end(), rhyme.begin(), ::tolower);
		l.append_attribute("rhyme") = rhyme.c_str();
	}

	// Add metric stress of syllables:
	string linemet = makeLineMet(line.syllables);
	l.append_attribute("met") = linemet.c_str();

	// Add number of syllables in line:
	if (line.syllables.size() == 7) {
		l.append_attribute("n") = "settenario";
	} else if (line.syllables.size() == 11) {
		l.append_attribute("n") = "endecasillabo";
	} else {
		l.append_attribute("n") = to_string(line.syllables.size()).c_str();
	}

	// Enjambments are indiciated by a + sign after the "=" sign at the end of the poetic line.
	// Alternately, enjambments can be categorized as "weak" or "strong" rather than "yes".
	// Currently Tasso poem markup is only using "yes" there is an enjambent rather than
	// qualifying by grammatical strength of the enjambment.
	HumRegex hre;
	if (hre.search(line.rest, "\\+")) {
		l.append_attribute("enjamb") = "yes";
	}

	// Embed the original input (for debugging):
	xml_node comment = l.append_child(node_comment);
	string text = " INPUT: ";
	string input = line.input;
	hre.replaceDestructive(input, "", "^\\s+");
	hre.replaceDestructive(input, "", "\\s+$");
	hre.replaceDestructive(input, " =", "\\s+=");
	text += input;
	text += " ";
	comment.set_value(text.c_str());

	for (int i=0; i<(int)line.syllables.size(); i++) {
		addSyllableToLine(l, line.syllables.at(i));
	}
}



//////////////////////////////
//
// makeLineMet --
//

string makeLineMet(vector<Syllable>& syllables) {
	string output;
	for (int i=0; i<(int)syllables.size(); i++) {
		if (syllables[i].accent.empty()) {
			output += "-";
		} else {
			output += syllables[i].accent[0];
		}
	}
	return output;
}



//////////////////////////////
//
// addSyllableToLine --
//

void addSyllableToLine(xml_node& line, Syllable& syllable) {
	xml_node seg = line.append_child("seg");

	// Indicate that the <seg> represents a poetic syllable:
	seg.append_attribute("type") = "syl";

	// Indicate the metric stress (linguistic stress actually):
	if (syllable.accent.empty()) {
		seg.append_attribute("met") = "-";
	} else {
		seg.append_attribute("met") = syllable.accent.c_str();
	}

	parseElisionsInSyllable(seg, syllable.text);
}


//////////////////////////////
//
// parseElisonInSyllable --
//     Example of elision: <seg met="+">ci<c met="_"> </c>as</seg>
//     For ci_as
//

void parseElisionsInSyllable(xml_node seg, const string& text) {
	HumRegex hre;
	if (!hre.search(text, "_")) {
		// no elision to markup
		seg.text().set(text.c_str());
		return;
	}

	vector<string> newtext;
	hre.split(newtext, text, "_");

	for (int i=0; i<(int)newtext.size(); i++) {
		seg.append_child(pugi::node_pcdata).set_value(newtext[i].c_str());
		if (i < (int)newtext.size() - 1) {
			xml_node c = seg.append_child("c");
			c.append_child(node_pcdata).set_value(" ");
			c.append_attribute("met") = "_";
		}
	}
}



//////////////////////////////
//
// printTeiDocument -- print PugiXML document.
//

void printTeiDocument(xml_document& tei, ostream& out) {
	tei.save(out, "\t");
}



