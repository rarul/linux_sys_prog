#include  <stdio.h>
#include  <err.h>
#include  <unistd.h>




int main(int argc, char *argv[]){
	//optarg
	//optind
	//opterr
	//optopt

	int c;
	int flag_a = 0;
	const char *arg_b = NULL;
	const char *arg_A = NULL;
	int flag_B = 0;
	while ((c=getopt(argc, argv, "ab:A:B")) != -1) {
		switch (c) {
			case 'a':
				flag_a++;
				break;
			case 'b':
				arg_b = optarg;
				//arg_b = strdup(optarg);
				break;
			case 'A':
				arg_A = optarg;
				//arg_A = strdup(optarg);
				break;
			case 'B':
				flag_B++;
				break;
			default:
				errx(1, "Invald arg: %d", c);
				break;
		}
	}

	printf("flag_a is %d\n", flag_a);
	printf(" arg_b is %s\n", arg_b);
	printf(" arg_A is %s\n", arg_A);
	printf("flag_B is %d\n", flag_B);

	printf("remained cmdline\n");
	for(c=optind; c<argc; c++) {
		printf("\t%s\n", argv[c]);
	}
	return 0;
}
