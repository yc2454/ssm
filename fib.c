#include <stdio.h>
#include <stdlib.h>
#include "ssm.h"

/* From my "runtime" PLT lecture 

int fib(int n) {
    int tmp1, tmp2, tmp3;
    tmp1 = n < 2;
    if (!tmp1) goto L1;
    return 1;
L1: tmp1 = n - 1;
    tmp2 = fib(tmp1);
L2: tmp1 = n - 2;
    tmp3 = fib(tmp1);
L3: tmp1 = tmp2 + tmp3;
    return tmp1;
}

0 1 2 3 4 5  6  7  8  9 10  11  12  13
1 1 2 3 5 8 13 21 34 55 89 144 233 377

 */
typedef struct {
	ACTIVATION_RECORD_FIELDS;

	ptr_int_cvt result;		// Where we should write our result
	int n, tmp1;
	int_cvt tmp2, tmp3;		// Local variables
} fib_act_t;

stepf_t step_fib;

fib_act_t *enter_fib(rar_t *cont, priority_t priority,
		depth_t depth, ptr_int_cvt result, int n)
{
	fib_act_t *act = (fib_act_t *) enter(sizeof(fib_act_t), step_fib, cont,
			priority, depth);
	act->n = n;
	act->result = result;
	initialize_int(&act->tmp2, 0);
	initialize_int(&act->tmp3, 0);
	return act;
}

void step_fib(rar_t *cont)  
{
	fib_act_t *act = (fib_act_t *) cont;
	/* printf("fib_step @%d n=%d\n", cont->pc, act->n); */
	switch (act->pc) {
	case 0:
		act->tmp1 = act->n < 2;			// tmp1 = n < 2
		if (!act->tmp1)				// if (!tmp1)
			goto L1;			//   goto L1;
		PTR_ASSIGN(act->result, act->priority, 1);
		leave((rar_t *) act, sizeof(fib_act_t));
		return;

	L1:
		act->tmp1 = act->n - 1;			// tmp1 = n - 1
		act->pc = 1;				// tmp2 = fib(tmp1)
		call((rar_t *) enter_fib((rar_t *) act, act->priority, act->depth,
				ptr_of_int(&act->tmp2), act->tmp1));
		return;

	case 1: // L2:
		act->tmp1 = act->n - 2;			// tmp1 = n - 2
		act->pc = 2;				// tmp3 = fib(tmp1)
		call((rar_t *) enter_fib((rar_t *) act, act->priority, act->depth,
				ptr_of_int(&act->tmp3), act->tmp1));
		return;

	case 2: // L3:
		act->tmp1 = act->tmp2.value + act->tmp3.value;
		PTR_ASSIGN(act->result, act->tmp1, act->priority);
		leave((rar_t *) act, sizeof(fib_act_t));
		return;
	}
}

void top_return(rar_t *cont)
{
	return;
}

int main(int argc, char *argv[])
{
	int_cvt result;
	initialize_int(&result, 0);
	int n = argc > 1 ? atoi(argv[1]) : 3;

	rar_t top = { .step = top_return };
	call((rar_t *) enter_fib(&top, PRIORITY_AT_ROOT, DEPTH_AT_ROOT,
			   ptr_of_int(&result), n));

	printf("%d\n", result.value);

	return 0;
}
