#include <lib.h>

int main() {
	int r;
	int n = 0;
	char buf[512];
	int id = 0;

	

	int fdnum;
	debugf("try open /newmotd\n");
	if ((r = open("/newmotd", O_RDWR)) < 0) {
		user_panic("open /newmotd: %d", r);
	}
	fdnum = r;
	debugf("/newmotd opened\n");

	debugf("offset now: %d\n",((struct Fd *)num2fd(fdnum))->fd_offset);

	id = fork();
	if(id == 0){
		debugf("I'm child\n");

		debugf("child offset now: %d\n",((struct Fd *)num2fd(fdnum))->fd_offset);
		while(n = read(fdnum, buf, 10) > 0)
			debugf("child offset now: %d\n",((struct Fd *)num2fd(fdnum))->fd_offset);
	}
	else{
		debugf("I'm father, my child is %d\n", id);
		debugf("father offset now: %d\n",((struct Fd *)num2fd(fdnum))->fd_offset);
		while(n = read(fdnum, buf, 10) > 0)
			debugf("father offset now: %d\n",((struct Fd *)num2fd(fdnum))->fd_offset);
	}

	if ((r = close(fdnum)) < 0) {
		user_panic("close /newmotd: %d", r);
	}
	debugf("/newmotd closed\n");
	return 0;
}
