# Generated by abnfc at Wed Jul 24 16:21:53 2024
# Output file: ./output/pol.rl
# Sources:
# 	core
# 	abnf
# 	./pol.abnf
%%{
	# write your name
	machine generated_from_abnf;

	# generated rules, define required actions
	ALPHA = 0x41..0x5a | 0x61..0x7a;
	BIT = "0" | "1";
	CHAR = 0x01..0x7f;
	CR = "\r";
	LF = "\n";
	CRLF = CR LF;
	CTL = 0x00..0x1f | 0x7f;
	DIGIT = 0x30..0x39;
	DQUOTE = "\"";
	HEXDIG = DIGIT | "A"i | "B"i | "C"i | "D"i | "E"i | "F"i;
	HTAB = "\t";
	SP = " ";
	WSP = SP | HTAB;
	LWSP = ( WSP | ( CRLF WSP ) )*;
	OCTET = 0x00..0xff;
	VCHAR = 0x21..0x7e;
	rulename = ALPHA ( ALPHA | DIGIT | "-" )*;
	comment = ";" ( WSP | VCHAR )* CRLF;
	c_nl = comment | CRLF;
	c_wsp = WSP | ( c_nl WSP );
	defined_as = c_wsp* ( "=" | "=/" ) c_wsp*;
	repeat = DIGIT+ | ( DIGIT* "*" DIGIT* );
	group = "(" c_wsp* alternation c_wsp* ")";
	option = "[" c_wsp* alternation c_wsp* "]";
	char_val = DQUOTE ( 0x20..0x21 | 0x23..0x7e )* DQUOTE;
	bin_val = "b"i BIT+ ( ( "." BIT+ )+ | ( "-" BIT+ ) )?;
	dec_val = "d"i DIGIT+ ( ( "." DIGIT+ )+ | ( "-" DIGIT+ ) )?;
	hex_val = "x"i HEXDIG+ ( ( "." HEXDIG+ )+ | ( "-" HEXDIG+ ) )?;
	num_val = "%" ( bin_val | dec_val | hex_val );
	prose_val = "<" ( 0x20..0x3d | 0x3f..0x7e )* ">";
	element = rulename | group | option | char_val | num_val | prose_val;
	repetition = repeat? element;
	concatenation = repetition ( c_wsp+ repetition )*;
	alternation = concatenation ( c_wsp* "/" c_wsp* concatenation )*;
	elements = alternation c_wsp*;
	rule = rulename defined_as elements c_nl;
	rulelist = ( rule | ( c_wsp* c_nl ) )+;
	Signature = 0x50.0x52.0x65.0x67;
	Version = 0x01;
	Header = Signature Version;
	IdCharacter = 0x20..0x5b | 0x5d..0x7e;
	Key = IdCharacter+;
	KeyPath = Key | ( KeyPath "\\" Key );
	ValueCharacter = SP | VCHAR;
	Value = ValueCharacter{1,259};
	Type = 0x01 | 0x02 | 0x03 | 0x04 | 0x05 | "\a" | "\v";
	Size = 0x00..0xff;
	Data = OCTET{,65535};
	Instruction = "[" KeyPath ";" Value ";" Type ";" Size ";" Data "]";
	Instructions = Instruction | ( Instructions Instruction );
	Body = Instructions;
	PolicyFile = Header Body;

	# instantiate machine rules
	main:= PolicyFile;
}%%
