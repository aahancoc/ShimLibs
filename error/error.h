enum errCode {
	///Non-errors
	errNOERR,	//No error.
	errUSR_ABORT,	//User initiated abort, rollback and get to main menu
	errUSR_QUIT,	//User exited program, rollback and exit program
	
	///Critical errors
	errCRIT_DBASE,	//Internal database error.
	errCRIT_FILESYS,	//Internal filesystem error.
	errCRIT_FUNCT,	//Function call that should not fail just failed (Super-generic)
	errCRIT_ARGMNT, //Invalid function argument
	errCRIT_MALLOC, //Malloc failed. Panic.
	
	///Requires user intervention
	errWNG_NOSPC,	//No space left in file for patches.
	errWNG_BADDIR,	//Directory does not exist or contain expected contents
	errWNG_BADFILE,	//Files does not exist or contain expected contents
	errWNG_CONFIG,	//Invalid/missing configuration file
	errWNG_READONLY,	//File or folder is read-only
};

void ErrCracker(enum errCode error);
void ErrNo2ErrCode();