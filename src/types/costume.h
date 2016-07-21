/**
	Costumes

	The runtime stores costumes in memory as either an uncompressed bitmap or vertex data
	that can be passed to OpenGL with the right shader to render the costume. PNGs are
	decompressed at load time before passing them to the runtime.

	Although SVGs aren't implemented yet, the plan is to parse the SVG, using NanoSVG or
	similar tools, into a set of vertex data, and render the vertex data using a shader on
	the GPU. This, in theory, should give SVG costumes the proper scalability and
	vector-iness. I'm not sure how many unimplemented features tools like NanoSVG have, and
	I definitely don't have the time to make my own SVG renderer.

	Costumes of one sprite are stored as an array of struct Costumes, which is UT hashable
	structure, so UT provides the access by name and prev/next costume, while the array
	allows indexing.
**/

struct Costume {
	bool isTexture; // true if this costume is already stored on the GPU as a texture
	union {
		GLuint textureHandle;
		// TODO: vector format
	} data;
	//uint16 i; // index of costume can be calculated
	UT_hash_handle hh;
};
typedef struct Costume Costume;
