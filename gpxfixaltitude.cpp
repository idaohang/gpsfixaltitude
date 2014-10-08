#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <string>
#include <string.h>

#include <rapidxml.hpp>
#include <rapidxml_print.hpp>

//48.233 m für Karlsuhe!

class GpxFixAltitude {

	public:

		GpxFixAltitude( int argc,  char* argv[] );
		virtual ~GpxFixAltitude();

	private:

		std::string loadFile( const char* path );
		void saveFile( const char* path );
		void fixGpx( double offset, bool writeComment = true );
		void restoreHeight();

		char* docstr;
		rapidxml::xml_document<> doc;

		const char* infile;
		double      offset;
		const char* outfile;
};


GpxFixAltitude::GpxFixAltitude( int argc, char* argv[] ) {
	try {
		if ( argc < 2 ) {
			std::cout << "Usage: " << argv[0] << " infile  [offset] [outfile]\n\n source: the Sourcefile \n offset: The offset the altitude will be recalculated\n dest  : the Destination-File\n\nIf no offset is given, the original values will be restored by the History\nThe offset for your location can be calculated at http://www.unavco.org/software/geodetic-utilities/geoid-height-calculator/geoid-height-calculator.html par example" << std::endl;
			exit( 0 );
		}

// 		std::cout << argc << std::endl;

		infile  = outfile = argv[1]; 	// Infile is first parameter
		if ( argc >= 3 ) {				// second parameter given
			offset = atof( argv[2] );	// second parameter is offset
			if ( offset == 0 ) {			// offset is 0 -> no offset
				outfile = argv[2];		// second parameter is path of outfile
			} else if ( argc == 4 ) {		// third  parameter is given
				outfile = argv[3];		// third  parameter is path of outfile
			}
		}

		std::cout  << "Loading data from " << infile << argc <<  std::endl;
		std::string xml = loadFile( infile );

		docstr = new char[xml.length() + 1];
		strcpy( docstr, xml.c_str() );

		std::cout  << "parsing  GPS-file" << std::endl;
		doc.parse<rapidxml::parse_no_data_nodes>( docstr );

		if ( offset == 0 ) {
			restoreHeight();
		} else {
			fixGpx( offset );
		}

		std::cout  << "\nWrite data to " << outfile << std::endl;
		saveFile( outfile );

	} catch ( const char* ex ) {
		std::cout << "ERROR: " << ex << std::endl;
	}
}

GpxFixAltitude::~GpxFixAltitude() {
	delete[] docstr;
}


void GpxFixAltitude::restoreHeight() {
	rapidxml::xml_node<>* oldNode;    // pointer to node (for deletion)
	double                offset = 0; // sum of all ancient offsets

	rapidxml::xml_node<>* gpx  = doc.first_node( "gpx" );
	rapidxml::xml_node<>* node = gpx->first_node( "gpsfixaltitude" );

	while ( node != 0 ) {
		offset += atof( node->first_attribute( "offset" )->value() );
		oldNode = node;
		node = node->next_sibling( "gpsfixaltitude" );
		gpx->remove_node( oldNode );
	}
	std::cout  <<    "Sum of old offsets: " << offset << "m" << std::endl;
	fixGpx( - offset, false );
}

void GpxFixAltitude::fixGpx( double offset, bool writeComment ) {
	std::cout  << "\nCorrecting height with d(h) = " << offset << " m.\n" << std::endl;

	int nodes = 0;

	rapidxml::xml_node<>* trk = doc.first_node( "gpx" );

	if ( trk == 0 ) // No gpx-Node found
		throw "No gpx-Node found\nIs ist really a GPX-file?";

	if ( writeComment ) { // write history-node
		std::cout << "Writing History" << std::endl;;
		rapidxml::xml_node<>* comment = doc.allocate_node( rapidxml::node_element, doc.allocate_string( "gpsfixaltitude" ) );
		comment->append_attribute( doc.allocate_attribute( doc.allocate_string( "offset" ), doc.allocate_string( std::to_string( offset ).c_str() ) ) );
		trk->prepend_node( comment );
	}

	trk = trk->first_node( "trk" );
	if ( trk == 0 ) // No track found
		throw "No trk-Node found\nIs it really a GPX-file?";

	rapidxml::xml_node<>* trkseg = trk->first_node( "trkseg" );

	while ( trkseg != 0 ) {
		rapidxml::xml_node<>* trkpt = trkseg->first_node( "trkpt" );
		while ( trkpt != 0 ) {
			rapidxml::xml_node<>* ele = trkpt->first_node( "ele" );
			std::string newEle = std::to_string( atof( ele->value() ) + offset );
			ele->value( doc.allocate_string( newEle.c_str() ) );
			++nodes;
			trkpt = trkpt->next_sibling( "trkpt" );
		}
		trkseg = trkseg->next_sibling( "trkseg" );
	}
	std::cout << nodes << " values corrected" << std::endl;;
}

void GpxFixAltitude::saveFile( const char* path ) {
	std::ofstream file( path );
	if ( !file.is_open() ) {
		throw "Could not open the destinationfile";
	} else {
		file << doc;
		file.close();
	}
}

std::string GpxFixAltitude::loadFile( const char* path ) {
	std::ifstream inFile;
	std::string xml;

	inFile.open( path );
	if ( !inFile.is_open() ) {
		throw "Could not open the sourcefile";
	} else {
		std::stringstream strStream;
		strStream << inFile.rdbuf();
		xml = strStream.str();
	}
	return xml;
}

int main( int argc,  char* argv[] ) {
	GpxFixAltitude* gfa = new GpxFixAltitude( argc, argv );
	delete gfa;
}
