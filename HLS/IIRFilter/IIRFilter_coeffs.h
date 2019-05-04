/* Bandpass non SOS
const double IIR_coeffs[IIR_TAPS*2*NUM_SOS] = {
		0.222615414047534,	0,	-1.33569248428520,	0,	3.33923121071301,	0,	-4.45230828095068,
		0,	3.33923121071301,	0,	-1.33569248428520,	0,	0.222615414047534, //b0-12
		1,	-1.96010654166355,	-1.30783011540332,	3.53602471035519,	1.84881294036498,	-3.78291148991242,	-1.63835212915797,
		2.21665883428394,	0.994483318362787,	-0.714002197231014,	-0.339399416924713,	0.0971277387518167,	0.0495583579403096 // a0-12
};
*/

// Bandpass reordering of coefficients, rms error 3.88
const double IIR_coeffs[IIR_TAPS*2*NUM_SOS] = {
		1,	-2,	1,	1,	-1.92394156787552,	0.939311298149237,
		1,	-2,	1,	1,	-1.82239100687540,	0.837468122663999,
		1,	-2,	1,	1,	-1.76182430832712,	0.776981570758687,
		1,	2,	1,	1,	1.40950760823877,	0.744145659240527,
		1,	2,	1,	1,	1.13494599228478,	0.415138660735878,
		0.222615414047534,	0.445230828095067,	0.222615414047534,	1,	1.00359674089094,	0.262468522394817
};

/*
// No gain
const double IIR_coeffs_1[IIR_TAPS*2*NUM_SOS] = {
		1,	2,	1,	1,	-0.408871739030851,	0.0612308522852448,
		1,	2,	1,	1,	-0.423531164568301,	0.225709867542097,
		1,	2,	1,	1,	-0.511227426679672,	0.630765521935839,
		1,	-2,	1,	1,	-1.87240780953290,	0.876812440322984,
		1,	-2,	1,	1,	-1.91155123750821,	0.915653300483277,
		1,	-2,	1,	1,	-1.96642780809933,	0.970354512501061
};

const double IIR_gain = 0.00807976230159289;
const double IIR_gain6 = 0.4479536683245863313;

// Bandpass reordering of coefficients, rms error 10.9
const double IIR_coeffs_2[IIR_TAPS*2*NUM_SOS] = {
		1,	-2,	1,	1,	-1.96642780809933,	0.970354512501061,  // SOS5
		1,	-2,	1,	1,	-1.91155123750821,	0.915653300483277, // SOS4
		1,	-2,	1,	1,	-1.87240780953290,	0.876812440322984, // SOS3
		1,	 2,	1,	1,	-0.511227426679672,	0.630765521935839, // SOS2
		0.00807976230159289,0.0161595246031858,	0.00807976230159289, 1, -0.408871739030851,	0.0612308522852448, //SOS0
		1,	 2,	1,	1, 	-0.423531164568301,	0.225709867542097 // SOS1
};

// Bandpass reordering of coefficients, best rms error 10.83
const double IIR_coeffs_3[IIR_TAPS*2*NUM_SOS] = {
		1,	-2,	1,	1,	-1.96642780809933,	0.970354512501061,  // SOS5
		1,	-2,	1,	1,	-1.91155123750821,	0.915653300483277, // SOS4
		1,	-2,	1,	1,	-1.87240780953290,	0.876812440322984, // SOS3
		1,	 2,	1,	1,	-0.511227426679672,	0.630765521935839, // SOS2
		1,	 2,	1,	1, 	-0.423531164568301,	0.225709867542097, // SOS1
		0.00807976230159289,0.0161595246031858,	0.00807976230159289, 1, -0.408871739030851,	0.0612308522852448 //SOS0
};

// Bandpass filter
const double IIR_coeffs_4[IIR_TAPS*2*NUM_SOS] = {
		0.00807976230159289,   //b0, SOS0
		0.0161595246031858,
		0.00807976230159289,
		1,					   //a0, SOS0
		-0.408871739030851,
		0.0612308522852448,
		1,					   //b0, SOS1
		2,
		1,
		1,   				   //a0, SOS1
		-0.423531164568301,
		0.225709867542097,
		1,	 2,	1,	1,	-0.511227426679672,	0.630765521935839, // SOS2
		1,	-2,	1,	1,	-1.87240780953290,	0.876812440322984, // SOS3
		1,	-2,	1,	1,	-1.91155123750821,	0.915653300483277, // SOS4
		1,	-2,	1,	1,	-1.96642780809933,	0.970354512501061  // SOS5
};



// 2xNotch filter for test
const double IIR_coeffs_5[IIR_TAPS*2*NUM_SOS] = {
		1,	-1.00000000000000,	1, // b0, b1, b2 - SOS0
		1,	-0.100000000000000,	0.0100000000000000, // a0, a1, a2 - SOS0
		1,	-1.00000000000000,	1, // b0, b1, b2 - SOS1
		1,	-0.100000000000000,	0.0100000000000000, // a0, a1, a2 - SOS0
		1,	-1.00000000000000,	1, // b0, b1, b2 - SOS2
		1,	-0.100000000000000,	0.0100000000000000, // a0, a1, a2 - SOS0
		1,	-1.00000000000000,	1, // b0, b1, b2 - SOS3
		1,	-0.100000000000000,	0.0100000000000000, // a0, a1, a2 - SOS0
		1,	-1.00000000000000,	1, // b0, b1, b2 - SOS4
		1,	-0.100000000000000,	0.0100000000000000, // a0, a1, a2 - SOS0
		1,	-1.00000000000000,	1, // b0, b1, b2 - SOS5
		1,	-0.100000000000000,	0.0100000000000000 // a0, a1, a2 - SOS0
};

*/

