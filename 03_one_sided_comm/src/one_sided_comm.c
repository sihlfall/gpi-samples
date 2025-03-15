#include "gassert.h"
#include "success_or_die.h"
#include <GASPI.h>
#include <GASPI_Ext.h>
#include <stdlib.h>

static void wait_for_queue_entries (gaspi_queue_id_t * queue, int wanted_entries) {
    gaspi_number_t queue_size_max, queue_size, queue_num;
    SUCCESS_OR_DIE( gaspi_queue_size_max(&queue_size_max) )
    SUCCESS_OR_DIE( gaspi_queue_size(*queue, &queue_size) )
    SUCCESS_OR_DIE( gaspi_queue_num(&queue_num) )

    if (! (queue_size + wanted_entries <= queue_size_max)) {
        *queue = (*queue + 1) % queue_num;
        SUCCESS_OR_DIE( gaspi_wait (*queue, GASPI_BLOCK) )
    }
}

static void wait_for_queue_entries_for_write_notify (gaspi_queue_id_t * queue_id) {
    wait_for_queue_entries (queue_id, 2);
}

static void wait_for_flush_queues () {
    gaspi_number_t queue_num;
    SUCCESS_OR_DIE( gaspi_queue_num (&queue_num) )
    for (gaspi_queue_id_t queue = 0; queue < queue_num; ++queue) {
        SUCCESS_OR_DIE( gaspi_wait (queue, GASPI_BLOCK) )
    }
}

static void wait_or_die (
    gaspi_segment_id_t segment_id,
    gaspi_notification_id_t notification_id,
    gaspi_notification_t expected
) {
    gaspi_notification_id_t id;
    SUCCESS_OR_DIE( gaspi_notify_waitsome (
        segment_id, notification_id, 1, &id, GASPI_BLOCK
    ) )
    ASSERT( id == notification_id )
    gaspi_notification_t value;
    SUCCESS_OR_DIE( gaspi_notify_reset (segment_id, id, &value) )
}

#define LEFT(iProc,nProc) (((iProc) + (nProc) - 1) % (nProc))
#define RIGHT(iProc,nProc) (((iProc) + (nProc) + 1) % (nProc))

int main(int argc, char * argv[]) {
    static const int VLEN = 4;
    SUCCESS_OR_DIE( gaspi_proc_init (GASPI_BLOCK) )
    gaspi_rank_t iProc, nProc;
    SUCCESS_OR_DIE( gaspi_proc_rank (&iProc) )
    SUCCESS_OR_DIE( gaspi_proc_num (&nProc) )
    gaspi_segment_id_t const segment_id = 0;
    gaspi_size_t const segment_size = 2 * VLEN * sizeof (double);

    SUCCESS_OR_DIE( gaspi_segment_create (
        segment_id, segment_size,
        GASPI_GROUP_ALL, GASPI_BLOCK, GASPI_MEM_UNINITIALIZED
    ) )
    gaspi_pointer_t array;
    SUCCESS_OR_DIE( gaspi_segment_ptr (segment_id, &array) )
    double * src_array = (double *)array;
    double * dest_array = src_array + VLEN;

    for (int j = 0; j < VLEN; ++j) {
        src_array [j] = (double) (iProc * VLEN + j);
    }

    gaspi_notification_id_t data_available = 0;
    gaspi_queue_id_t queue_id = 0;
    gaspi_offset_t loc_off = 0;
    gaspi_offset_t rem_off = VLEN * sizeof (double);

    wait_for_queue_entries_for_write_notify (&queue_id);

    SUCCESS_OR_DIE( gaspi_write_notify (
        segment_id, loc_off, RIGHT(iProc, nProc),
        segment_id, rem_off,
        VLEN * sizeof (double),
        data_available, 1 + iProc, queue_id,
        GASPI_BLOCK
    ) )
    wait_or_die (segment_id, data_available, 1 + LEFT(iProc, nProc));

    for (int j = 0; j < VLEN; ++j) {
        gaspi_printf("rank %d elem %d %f\n", iProc, j, dest_array[j]);
    }

    wait_for_flush_queues ();
    SUCCESS_OR_DIE( gaspi_proc_term (GASPI_BLOCK) )
    return EXIT_SUCCESS;
}