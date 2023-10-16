/*
 File: ContFramePool.C

 Author: Ashutosh Punyani
 Date  : Sept 15, 2023
 Description: Management of the CONTIGUOUS Free-Frame Pool.

 */

/*--------------------------------------------------------------------------*/
/*
 POSSIBLE IMPLEMENTATION
 -----------------------

 The class SimpleFramePool in file "simple_frame_pool.H/C" describes an
 incomplete vanilla implementation of a frame pool that allocates
 *single* frames at a time. Because it does allocate one frame at a time,
 it does not guarantee that a sequence of frames is allocated contiguously.
 This can cause problems.

 The class ContFramePool has the ability to allocate either single frames,
 or sequences of contiguous frames. This affects how we manage the
 free frames. In SimpleFramePool it is sufficient to maintain the free
 frames.
 In ContFramePool we need to maintain free *sequences* of frames.

 This can be done in many ways, ranging from extensions to bitmaps to
 free-lists of frames etc.

 IMPLEMENTATION:

 One simple way to manage sequences of free frames is to add a minor
 extension to the bitmap idea of SimpleFramePool: Instead of maintaining
 whether a frame is FREE or ALLOCATED, which requires one bit per frame,
 we maintain whether the frame is FREE, or ALLOCATED, or HEAD-OF-SEQUENCE.
 The meaning of FREE is the same as in SimpleFramePool.
 If a frame is marked as HEAD-OF-SEQUENCE, this means that it is allocated
 and that it is the first such frame in a sequence of frames. Allocated
 frames that are not first in a sequence are marked as ALLOCATED.

 NOTE: If we use this scheme to allocate only single frames, then all
 frames are marked as either FREE or HEAD-OF-SEQUENCE.

 NOTE: In SimpleFramePool we needed only one bit to store the state of
 each frame. Now we need two bits. In a first implementation you can choose
 to use one char per frame. This will allow you to check for a given status
 without having to do bit manipulations. Once you get this to work,
 revisit the implementation and change it to using two bits. You will get
 an efficiency penalty if you use one char (i.e., 8 bits) per frame when
 two bits do the trick.

 DETAILED IMPLEMENTATION:

 How can we use the HEAD-OF-SEQUENCE state to implement a contiguous
 allocator? Let's look a the individual functions:

 Constructor: Initialize all frames to FREE, except for any frames that you
 need for the management of the frame pool, if any.

 get_frames(_n_frames): Traverse the "bitmap" of states and look for a
 sequence of at least _n_frames entries that are FREE. If you find one,
 mark the first one as HEAD-OF-SEQUENCE and the remaining _n_frames-1 as
 ALLOCATED.

 release_frames(_first_frame_no): Check whether the first frame is marked as
 HEAD-OF-SEQUENCE. If not, something went wrong. If it is, mark it as FREE.
 Traverse the subsequent frames until you reach one that is FREE or
 HEAD-OF-SEQUENCE. Until then, mark the frames that you traverse as FREE.

 mark_inaccessible(_base_frame_no, _n_frames): This is no different than
 get_frames, without having to search for the free sequence. You tell the
 allocator exactly which frame to mark as HEAD-OF-SEQUENCE and how many
 frames after that to mark as ALLOCATED.

 needed_info_frames(_n_frames): This depends on how many bits you need
 to store the state of each frame. If you use a char to represent the state
 of a frame, then you need one info frame for each FRAME_SIZE frames.

 A WORD ABOUT RELEASE_FRAMES():

 When we releae a frame, we only know its frame number. At the time
 of a frame's release, we don't know necessarily which pool it came
 from. Therefore, the function "release_frame" is static, i.e.,
 not associated with a particular frame pool.

 This problem is related to the lack of a so-called "placement delete" in
 C++. For a discussion of this see Stroustrup's FAQ:
 http://www.stroustrup.com/bs_faq2.html#placement-delete

 */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "cont_frame_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

ContFramePool *ContFramePool::head = NULL;
ContFramePool *ContFramePool::current_pointer = NULL;

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   C o n t F r a m e P o o l */
/*--------------------------------------------------------------------------*/
//  get the state of a frame by frame_no
ContFramePool::FrameState ContFramePool::get_state(unsigned long _frame_no)
{
    unsigned int bitmap_index = _frame_no/4;
    unsigned int bitmap_shift = (_frame_no%4)*2;
    unsigned char bitmap_mask = (0x3) << bitmap_shift;
    unsigned char _state = (bitmap[bitmap_index] & bitmap_mask)>>bitmap_shift;


    switch (_state)
    {
    case 0x0:
        return FrameState::Free;
        break;
    case 0x1:
        return FrameState::Used;
        break;
    case 0x2:
        return FrameState::HoS;
        break;
    }
}

//  set the state of a frame by frame_no and state to be set
void ContFramePool::set_state(unsigned long _frame_no, FrameState _state)
{
    unsigned int bitmap_index = _frame_no/4;
    unsigned int bitmap_shift = (_frame_no%4)*2;
    unsigned char bitmap_mask = (0x3) << bitmap_shift;
    unsigned char free_mask = (0x0) << bitmap_shift;
    unsigned char hos_mask = (0x2) << bitmap_shift;
    unsigned char used_mask = (0x1) << bitmap_shift;

    switch(_state) {
      case FrameState::Free:
        bitmap[bitmap_index] = (bitmap[bitmap_index]& ~(bitmap_mask)) | free_mask;
        break;
      case FrameState::HoS:
        bitmap[bitmap_index] = (bitmap[bitmap_index]& ~(bitmap_mask)) | hos_mask;
        break;
      case FrameState::Used:
        bitmap[bitmap_index] = (bitmap[bitmap_index]& ~(bitmap_mask)) | used_mask;
        break;
    }
}

ContFramePool::ContFramePool(unsigned long _base_frame_no,
                             unsigned long _n_frames,
                             unsigned long _info_frame_no)
{
    base_frame_no = _base_frame_no;
    nframes = _n_frames;
    nFreeFrames = _n_frames;
    info_frame_no = _info_frame_no;

    // If _info_frame_no is zero then we keep management info in the first
    //frame, else we use the provided frame to keep management info
    if (info_frame_no == 0)
    {
        bitmap = (unsigned char *)(base_frame_no * FRAME_SIZE);
    }
    else
    {
        bitmap = (unsigned char *)(info_frame_no * FRAME_SIZE);
    }
    // Everything ok. Proceed to mark all frame as free.
    for (int fno = 0; fno < _n_frames; fno++)
    {
        set_state(fno, FrameState::Free);
    }

    // Mark the first frame as being used if it is being used
    if (_info_frame_no == 0)
    {
        set_state(0, FrameState::HoS);
        nFreeFrames--;
    }
    // used to keep track of pools
    if (head == NULL)
    {
        head = this;
        current_pointer = this;
    }
    else
    {
        current_pointer->next = this;
        current_pointer = this;
    }
    next=NULL;
}

// this function find frames to be allocated and allocate them
unsigned long ContFramePool::get_frames(unsigned int _n_frames)
{
    unsigned long frame_no = 0;
    unsigned long max_count_frames=0;
    unsigned long free_frame_start = 0;
    unsigned long frame_start = base_frame_no;

    if (_n_frames > nFreeFrames)
    {
        Console::puts("\nFree Frames not available\n ");
        assert(false);
        return 0;
    }

while(frame_no<nframes){
    if(get_state(frame_no)==FrameState::Free){
        max_count_frames++;
        if(max_count_frames==_n_frames){
            free_frame_start=frame_no-(_n_frames-1);
            break;
        }
    }
    else{
        max_count_frames=0;
    }
    frame_no++;
}
if(max_count_frames!=_n_frames){
    Console::puts("Continuous Free Frames not available\n");
            return 0;
        }else{
            frame_start=base_frame_no+free_frame_start;
    set_state(free_frame_start, FrameState::HoS);
    for (int fno = free_frame_start + 1; fno < (free_frame_start + _n_frames); fno++)
    {

        set_state(fno, FrameState::Used);
    }
    nFreeFrames -= _n_frames;

    return frame_start;
}
}

// this function marks frames to be un used

void ContFramePool::mark_inaccessible(unsigned long _base_frame_no,
                                      unsigned long _n_frames)
{


    set_state(_base_frame_no - this->base_frame_no, FrameState::HoS);
    for (int fno = _base_frame_no + 1; fno < _base_frame_no + _n_frames; fno++)
    {
        set_state(fno - this->base_frame_no, FrameState::Used);
    }
    nFreeFrames -= _n_frames;
}

// this function releases the frames from the particular pool

void ContFramePool::release_frame(unsigned long _start_frame_no)
{   

    unsigned long frame_no=_start_frame_no-base_frame_no;
    if (get_state(frame_no) != FrameState::HoS)
    {
         Console::puts("Frame is not the head of the contiguous frames\n");
        assert(false);
    }
    else
    {
        // relesase the frames by setting their state as free
        set_state(_start_frame_no, FrameState::Free);
    	nFreeFrames++;
    	unsigned long i = _start_frame_no + 1;

         while (get_state(i-base_frame_no) != FrameState::Free)
         {
             set_state(i, FrameState::Free);
             nFreeFrames++;
             i++;
         }
    }

}

// this function identifies from which pool to release the frames and call release frame to release

void ContFramePool::release_frames(unsigned long _first_frame_no)
{
    // to find the pool to which belongs to
    ContFramePool *iterator = head;
    while (iterator != NULL)
    {
        if (iterator->base_frame_no <= _first_frame_no && (iterator->base_frame_no + ((iterator->nframes) - 1))>= _first_frame_no)
        {
            break;
        }
       else
        {
            iterator = iterator->next;
       }
    }
    iterator->release_frame(_first_frame_no);
}

unsigned long ContFramePool::needed_info_frames(unsigned long _n_frames)
{
    unsigned long frame_storage_size = 4*FRAME_SIZE;
    return _n_frames / frame_storage_size + (_n_frames % frame_storage_size > 0 ? 1 : 0);
}
