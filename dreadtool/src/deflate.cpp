
#include "common.h"

#include "zlib.h"

#define ASSERT(x)		assert(x)


static uint32_t nr__lit, nr__len, tot__cplen, nr__codes, nr__codebits;
static uint32_t nr__longcodes;


static void errx(int status, const char *fmt, ...)
{
	va_list vl;

//	printf("errx errno=%d\n", errno);

	va_start(vl, fmt);
	vprintf(fmt, vl);
	va_end(vl);
}

struct deflate_stream {
	struct {
		uint8_t *p, *end;
	} in, out;
	uint32_t cur, nr; /* input bit tracking */
	uint32_t leeway;
};

static uint32_t deflate_stream_next_bits(
	struct deflate_stream *s, unsigned int nr)
{
	uint32_t w;
	while( s->nr < nr ) {
		ASSERT(s->in.p < s->in.end);
		s->cur |= *(s->in.p++) << s->nr;
		s->nr += 8;
	}
	w = s->cur & ((1u << nr) - 1);
	s->cur >>= nr;
	s->nr -= nr;
	return w;
}

/* Longest possible code. */
#define MAX_CODE_LEN   16

/* (Maximum) alphabet sizes. */
#define nr_codelen_symbols  19
#define nr_litlen_symbols  288
#define nr_distance_symbols 32

/* Order of code lengths for the code length alphabet. */
static const uint8_t codelen_order[nr_codelen_symbols] ={
	16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 };

/* Base values and extra bits for lit/len symbols 257+. */
static const uint16_t length_base[] ={
	3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
	35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258 };
static const uint8_t length_extrabits[] ={
	0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
	3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0 };

/* Base values and extra bits for distance symbols. */
static const uint16_t dist_base[] ={
	1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513,
	769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577 };
static const uint8_t dist_extrabits[] ={
	0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
	7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13 };

struct node {
	uint16_t l, r;
#define LEAF 0x1000
};

static uint32_t deflate_stream_next_symbol(
	struct deflate_stream *s, struct node *tree)
{
	struct node *node = tree;
	uint16_t idx, bits = 0;

	for( ;;) {
		bits++;
		idx = deflate_stream_next_bits(s, 1) ? node->r : node->l;
		if( idx & LEAF )
			break;
		if( idx == 0 )
			errx(1, "Bad symbol decode");
		node = &tree[idx];
	}

	nr__codebits += bits;
	nr__codes++;
	if( bits > 8 )
		nr__longcodes++;

	return idx & 0xfffu;
}

static void build_code(uint8_t *len, struct node *nodes, uint16_t nr_symbols)
{
	struct node *node;
	uint16_t *pnode, next_node;
	uint16_t i, code, mask, bl_count[MAX_CODE_LEN+1];
	uint16_t next_code[MAX_CODE_LEN+1]; /* XXX merge with bl_count */
	uint8_t l;

	memset(bl_count, 0, (MAX_CODE_LEN+1)*2);
	for( i = 0; i < nr_symbols; i++ )
		bl_count[len[i]]++;
	bl_count[0] = 0; /* zero-length codes are not included */

	code = 0;
	for( i = 1; i <= MAX_CODE_LEN; i++ ) {
		code = (code + bl_count[i-1]) << 1;
		next_code[i] = code;
	}

	memset(nodes, 0, nr_symbols*sizeof(*nodes));
	next_node = 1;
	for( i = 0; i < nr_symbols; i++ ) {
		if( !(l = len[i]) )
			continue;
		code = next_code[l]++;
		mask = 1u << (l - 1);
		node = &nodes[0];
		for( ;;) {
			pnode = (code & mask) ? &node->r : &node->l;
			if( !(mask >>= 1) )
				break;
			if( !*pnode )
				*pnode = next_node++;
			node = &nodes[*pnode];
		}
		*pnode = LEAF | i;
	}
}

static void huffman(struct deflate_stream *main, struct deflate_stream *prefix)
{
	uint16_t hlit, i, litlen_sym;
	uint8_t hdist, hclen, len[nr_litlen_symbols + nr_distance_symbols];
	struct node codelen_tree[nr_codelen_symbols];
	struct node litlen_tree[nr_litlen_symbols];
	struct node dist_tree[nr_distance_symbols];
	struct deflate_stream *s = prefix ? prefix : main;

	memset(codelen_tree, 0, sizeof(codelen_tree));
	memset(litlen_tree, 0, sizeof(litlen_tree));

	hlit = deflate_stream_next_bits(s, 5) + 257;
	ASSERT(hlit <= nr_litlen_symbols);
	hdist = deflate_stream_next_bits(s, 5) + 1;
	ASSERT(hdist <= nr_distance_symbols);
	hclen = deflate_stream_next_bits(s, 4) + 4;
	ASSERT(hclen <= nr_codelen_symbols);

	for( i = 0; i < hclen; i++ )
		len[codelen_order[i]] = deflate_stream_next_bits(s, 3);
	for( ; i < nr_codelen_symbols; i++ )
		len[codelen_order[i]] = 0;
	build_code(len, codelen_tree, nr_codelen_symbols);

	/* Literal & distance code lengths stored in a single coded stream. */
	for( i = 0; i < hlit+hdist;) {
		switch( len[i] = deflate_stream_next_symbol(s, codelen_tree) ) {
		case 16: {
			uint8_t l = len[i-1], rep = deflate_stream_next_bits(s, 2) + 3;
			while( rep-- ) len[i++] = l;
			break;
		}
		case 17: {
			uint8_t rep = deflate_stream_next_bits(s, 3) + 3;
			while( rep-- ) len[i++] = 0;
			break;
		}
		case 18: {
			uint8_t rep = deflate_stream_next_bits(s, 7) + 11;
			while( rep-- ) len[i++] = 0;
			break;
		}
		default: /* 0..15 */
			i++;
			break;
		}
	}
	build_code(&len[0], litlen_tree, hlit);
	build_code(&len[hlit], dist_tree, hdist);

	s = main;
	while( (litlen_sym = deflate_stream_next_symbol(s, litlen_tree)) != 256 ) {
		int32_t overtake;
		uint16_t dist_sym, cplen, cpdst;
		uint8_t *p;

		if( litlen_sym < 256 ) {
			ASSERT(s->out.p < s->out.end);
			*(s->out.p++) = (uint8_t)litlen_sym;
			nr__lit++;
		}
		else {
			/* 257+ */
			nr__len++;
			cplen = length_base[litlen_sym-257] +
				deflate_stream_next_bits(s, length_extrabits[litlen_sym-257]);
			dist_sym = deflate_stream_next_symbol(s, dist_tree);
			cpdst = dist_base[dist_sym] +
				deflate_stream_next_bits(s, dist_extrabits[dist_sym]);
			p = s->out.p - cpdst;
			ASSERT((s->out.p + cplen) <= s->out.end);
			tot__cplen += cplen;
			while( cplen-- )
				*(s->out.p++) = *p++;
		}
		/* Calculate the amount by which the output has overtaken the
		* remaining input stream, if the input stream were located at the
		* far end of the output buffer. Track the max of that value. */
		overtake = (s->in.end - s->in.p) - (s->out.end - s->out.p);
		if( overtake > (int32_t)s->leeway )
			s->leeway = overtake;
	}
}

static void uncompressed_block(struct deflate_stream *s)
{
	uint16_t len, nlen;
	s->cur = s->nr = 0; /* byte boundary */
	len = deflate_stream_next_bits(s, 16);
	nlen = deflate_stream_next_bits(s, 16);
	ASSERT(nlen == (uint16_t)~len);
	ASSERT((s->in.p + len) <= s->in.end);
	ASSERT((s->out.p + len) <= s->out.end);
	memcpy(s->out.p, s->in.p, len);
	s->in.p += len;
	s->out.p += len;
}

static void deflate_process_block(struct deflate_stream *s)
{
	uint8_t bfinal, btype;

	do {
		bfinal = deflate_stream_next_bits(s, 1);
		btype = deflate_stream_next_bits(s, 2);

		switch( btype ) {
		case 0:
			uncompressed_block(s);
			break;
		case 1: {
			static const uint8_t static_prefix[] ={
				0xff, 0x5b, 0x00, 0x6c, 0x03, 0x36, 0xdb,
				0xb6, 0x6d, 0xdb, 0xb6, 0x6d, 0xdb, 0xb6,
				0xcd, 0xdb, 0xb6, 0x6d, 0xdb, 0xb6, 0x6d,
				0xdb, 0xa8, 0x6d, 0xce, 0x8b, 0x6d, 0x3b };
			struct deflate_stream dfls_prefix;
			memset(&dfls_prefix, 0, sizeof(struct deflate_stream));
			dfls_prefix.in.p = (uint8_t *)static_prefix;
			dfls_prefix.in.end = (uint8_t *)static_prefix +
				sizeof(static_prefix);
			huffman(s, &dfls_prefix);
			break;
		}
		case 2:
			huffman(s, NULL);
			break;
		default:
			errx(1, "Unknown DEFLATE block type %u", btype);
		}
	} while( !bfinal );
}



int run_deflate(const uint8_t *data, int length, vector<byte> &out_buff, int *out_leeway)
{
	int ret;

	out_buff.clear();

	// ================================ Init deflate ================================
	z_stream stream ={};
	ret = deflateInit(&stream, 9);	// maximum compression
	if( ret != Z_OK )
	{
		printf("deflateInit: error code %d\n", ret);
		return ret;
	}

	// ================================ Setup source data ================================
	stream.next_in = (unsigned char *)data;
	stream.avail_in = (unsigned int)length;


	// ================================ Deflate ================================
	while( 1 )
	{
		// reserve output buffer
		out_buff.resize(stream.total_out + 16*1024);
		stream.next_out = &out_buff[stream.total_out];
		stream.avail_out = out_buff.size() - stream.total_out;

		ret = deflate(&stream, Z_FINISH);
		if( ret == Z_STREAM_END )
			break;

		if( ret != Z_OK )
		{
			printf("deflate: error code %d\n", ret);
			out_buff.clear();
			return ret;
		}
	}

	// ================================ Deflate end ================================
	ret = deflateEnd(&stream);
	if( ret != Z_OK )
	{
		printf("deflateEnd: error code %d\n", ret);
		out_buff.clear();
		return ret;
	}

	// ================================ Trim header & CRC ================================
	if( stream.total_out < 6 )
	{
		printf("deflate: bad stream\n");
		out_buff.clear();
		return Z_STREAM_ERROR;
	}

	out_buff.resize(stream.total_out-4);
	out_buff.erase(out_buff.begin(), out_buff.begin()+2);


	// ================================ Unpack (compute leeway) ================================
	if( out_buff.size()>0 && out_leeway )
	{
		vector<uint8_t> tmp;
		tmp.resize(length);

		deflate_stream dfls ={};
		dfls.in.p = &out_buff[0];
		dfls.in.end = &out_buff[0] + out_buff.size();
		dfls.out.p = &tmp[0];
		dfls.out.end = &tmp[0] + length;
		deflate_process_block(&dfls);
		ASSERT(dfls.in.p == dfls.in.end);
		ASSERT(dfls.out.p == dfls.out.end);

		*out_leeway = dfls.leeway;
	}


	return ret;
}
