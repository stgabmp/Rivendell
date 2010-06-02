int encode_init(frame_info *frame);
void scalefactor_calc_new (double sb_sample[][3][SCALE_BLOCK][SBLIMIT],
			   unsigned int scalar[][3][SBLIMIT], int nch,
			   int sblimit);

INLINE double mod (double a);

void combine_LR_new (double sb_sample[2][3][SCALE_BLOCK][SBLIMIT],
		     double joint_sample[3][SCALE_BLOCK][SBLIMIT], int sblimit);

void find_sf_max (unsigned int sf_index[2][3][SBLIMIT], frame_info * frame,
		  double sf_max[2][SBLIMIT]);

void sf_transmission_pattern (unsigned int sf_index[2][3][SBLIMIT],
			      unsigned int sf_selectinfo[2][SBLIMIT],
			      frame_info * frame);

void write_header (frame_info * frame, Bit_stream_struc * bs);

void write_bit_alloc (unsigned int bit_alloc[2][SBLIMIT],
		      frame_info * frame, Bit_stream_struc * bs);

void write_scalefactors (unsigned int bit_alloc[2][SBLIMIT],
			 unsigned int sf_selectinfo[2][SBLIMIT],
			 unsigned int scalar[2][3][SBLIMIT], frame_info * frame,
			 Bit_stream_struc * bs);

void subband_quantization_new (unsigned int sf_index[2][3][SBLIMIT],
		      double sb_samples[2][3][SCALE_BLOCK][SBLIMIT],
		      unsigned int j_scale[3][SBLIMIT],
		      double j_samps[3][SCALE_BLOCK][SBLIMIT],
		      unsigned int bit_alloc[2][SBLIMIT],
		      unsigned int sbband[2][3][SCALE_BLOCK][SBLIMIT],
			  frame_info * frame);

void write_samples_new (unsigned int sbband[2][3][SCALE_BLOCK][SBLIMIT],
		      unsigned int bit_alloc[2][SBLIMIT],
			frame_info * frame, Bit_stream_struc * bs);

/*******************************************************
   Bit Allocation Stuff 
******************************************************/

int bits_for_nonoise_new (double SMR[2][SBLIMIT],
			  unsigned int scfsi[2][SBLIMIT], frame_info * frame, float min_mnr,
			  unsigned int bit_alloc[2][SBLIMIT]);
void main_bit_allocation_new (double SMR[2][SBLIMIT],
			  unsigned int scfsi[2][SBLIMIT],
			  unsigned int bit_alloc[2][SBLIMIT], int *adb,
			  frame_info * frame, options * glopts);
void VBR_maxmnr_new (double mnr[2][SBLIMIT], char used[2][SBLIMIT], int sblimit,
		 int nch, int *min_sb, int *min_ch, options * glopts);
int
VBR_bit_allocation_new (double SMR[2][SBLIMIT],
		    unsigned int scfsi[2][SBLIMIT],
		    unsigned int bit_alloc[2][SBLIMIT], int *adb,
		    frame_info * frame, options * glopts);
void maxmnr_new (double mnr[2][SBLIMIT], char used[2][SBLIMIT], int sblimit,
	     int nch, int *min_sb, int *min_ch);
int a_bit_allocation_new (double SMR[2][SBLIMIT],
		      unsigned int scfsi[2][SBLIMIT],
		      unsigned int bit_alloc[2][SBLIMIT], int *adb,
		      frame_info * frame);
