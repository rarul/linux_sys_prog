#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <err.h>
#include  <getopt.h>



static const char *buildup_optstring(const struct option longopts[]) {
	const int retval_length = 64;
	char *retval = malloc(retval_length);
	char *p = NULL;
	int i;

	memset (retval, 0, retval_length);
	p = retval;
	for( i=0; longopts[i].name; i++) {
		*p = longopts[i].val;
		p++;
		if (longopts[i].has_arg != no_argument) {
			*p = ':';
			p++;
		}
	}
	return retval;
}
int main(int argc, char *argv[]){
	//optarg
	//optind
	//opterr
	//optopt

	const struct option longopts[] = {
		{"add",    no_argument,       NULL, 'a'},
		{"break",  no_argument,       NULL, 'b'},
		{"clear",  required_argument, NULL, 'c'},
		{"delete", optional_argument, NULL, 'd'},
		{    NULL, 0,                 NULL,  0 },
	};


	int c, longindex;
	int flag_a = 0;
	int flag_b = 0;
	const char *arg_c = NULL;
	const char *arg_d = NULL;

	const char *optstring = buildup_optstring(longopts);

	while ((c=getopt_long(argc, argv, optstring, longopts, &longindex)) != -1) {
		switch (c) {
			case 'a':
				flag_a++;
				break;
			case 'b':
				flag_b++;
				//arg_b = strdup(optarg);
				break;
			case 'c':
				arg_c = optarg;
				//arg_c = strdup(optarg);
				break;
			case 'd':
				arg_d = optarg;
				//arg_d = strdup(optarg);
				break;
			default:
				errx(1, "Invald arg: %d", c);
				break;
		}
	}

	printf("flag_a is %d\n", flag_a);
	printf("flag_b is %d\n", flag_b);
	printf(" arg_c is %s\n", arg_c);
	printf(" arg_d is %s\n", arg_d);

	printf("remained cmdline\n");
	for(c=optind; c<argc; c++) {
		printf("\t%s\n", argv[c]);
	}

	printf("original cmdline\n");
	for(c=0; c<argc; c++) {
		printf("\t%s\n", argv[c]);
	}
return 0;
}
