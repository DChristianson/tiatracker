; =====================================================================
; Flags
; =====================================================================

TT_SPEED                = 10    ; duration (number of TV frames) of a note
TT_ODD_SPEED            = 10    ; duration of odd frames (needs TT_USE_FUNKTEMPO)

TT_USE_OVERLAY          = 1     ; 1: Overlay percussion, +40 bytes
TT_USE_SLIDE            = 1     ; 1: Melodic instrument slide, +11 bytes
TT_USE_GOTO             = 1     ; 1: Goto pattern, +8 bytes
TT_USE_FUNKTEMPO        = 1     ; 1: Odd/even rows have different SPEED values, +7 bytes
; If the very first notes played on each channel are not PAUSE, HOLD or
; SLIDE, i.e. if they start with an instrument or percussion, then set
; this flag to 0 to save 2 bytes
TT_STARTS_WITH_NOTES    = 0     ; 0: +2 bytes


; =====================================================================
; Permanent variables. These are states needed by the player.
; =====================================================================
tt_timer                ds 1    ; current music timer value
tt_cur_pat_index_c0     ds 1    ; current pattern index into tt_SequenceTable
tt_cur_pat_index_c1     ds 1
tt_cur_note_index_c0    ds 1    ; note index into current pattern
tt_cur_note_index_c1    ds 1
tt_envelope_index_c0    ds 1    ; index into ADSR envelope
tt_envelope_index_c1    ds 1
tt_cur_ins_c0           ds 1    ; current instrument
tt_cur_ins_c1           ds 1


; =====================================================================
; Temporary variables. These will be overwritten during a call to the
; player routine, but can be used between calls for other things.
; =====================================================================
tt_ptr                  ds 2