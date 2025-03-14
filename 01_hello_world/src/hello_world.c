#include "success_or_die.h"
#include <GASPI.h>
#include <GASPI_Ext.h>
#include <stdlib.h>

int main(int argc, char * argv[]) {
    SUCCESS_OR_DIE( gaspi_proc_init (GASPI_BLOCK) )

    gaspi_rank_t rank, num;
    SUCCESS_OR_DIE( gaspi_proc_rank(&rank) )
    SUCCESS_OR_DIE( gaspi_proc_num(&num) )

    gaspi_printf ("Hello world from rank %d of %d\n", rank, num);

    SUCCESS_OR_DIE( gaspi_proc_term (GASPI_BLOCK) )
    return EXIT_SUCCESS;
}