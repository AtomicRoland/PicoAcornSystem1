; Clock - a clock demonstration program

        org &200
        dismode = &0e   ; display mode
        disbuf = &10    ; display buffer
        time = &80      ; base address of the time
        date = &88      ; base address of the date
        seconds = &90   ; seconds counter
        binday = &91    ; day part of date in binary format
        binmonth = &92  ; month part of date in binary format
        mode = &93      ; display mode (0 = time, &ff = date)
        timerl = &94    ; low byte of timer
        timerh = &95    ; high byte of timer
        timerv = &12d   ; timer value  (NOTE: high byte must minimal be &01 !)

        temperature = &0e28 ; temperature value
        hwtimer = &0e2c     ; hardware timer, decrements every 10 ms

        font = &ffea    ; font table (digits)
        display = &fe0c ; monitor display routine and read key. Pressed key is returned in A. 
                        ; keys 16 & 17 up/down, key 13 is S, hex keys contain their value
                        ; set &000E to &00 to do single scan (A=&00 if no key pressed)
        init = &fefb    ; return to monitor
        oswait = &fecd  ; 3280 us wait routine

;        org &2000-22
;.atmheader
;        equs "clock", 0,0,0,0,0,0,0,0,0,0,0
;        equw begin
;        equw begin
;        equw end-begin

.begin  jsr settimer    ; set the timer
        jsr setclock    ; clear the time and date (set to 00.00 0000.00.00)
        sta temperature ; performs temperature setup
.loop1  sta dismode     ; A=0 after clear; set scan mode to single scan
        sta mode        ; set display mode to time
.loop2  lda mode        ; get mode
        bne loop3       
        jsr displayTime ; display the time
        jmp loop4
.loop3  jsr displayDate ; display the date      

.loop4  cmp #&16        ; test if cursur up/down is pressed
        beq setupdown   ; jump if cursor up is pressed
        cmp #&17
        beq setupdown   ; jump if cursor down is pressed
        cmp #&13        ; test if S key is pressed
        bne loop5       ; jump if not S
        jmp setclock    ; jump to set time and date
.loop5  jsr updateTime

        jmp loop2       ; display the time
         


.displayTime
        ldx #3          ; load counter
.dt1    ldy time,x      ; load digit from time
        lda font,y      ; load display data
        sta disbuf,x    ; store in display buffer
        dex             ; decrement x
        bpl dt1
        lda time        ; if hours high is zero
        bne dt2
        lda #0          ; then don't display the digit
        sta disbuf
.dt2    lda seconds     ; load seconds
        lsr a           ; shift low bit to carry
        bcc displayTemp ; if clear then jump
        lda disbuf+1    ; load second digit
        ora #&80        ; turn decimal point on (seconds indicator)
        sta disbuf+1    ; store it back

.displayTemp
        lda temperature ; load control byte for temperature
        cmp #'T'        ; is temperature enabled?
        bne dt6         ; no, then jump to skip temperature    
        ldx #2          ; load counter
.dt3    ldy temperature+1,x  ; load temperature data
        lda font,y      ; load font data
        cpx #1          ; test for second digit
        bne dt4         ; jump if not second digit
        ora #&80        ; turn decimal point on (temp separator)
.dt4    sta disbuf+5,x  ; store in display buffer
        dex             ; decrement x
        bpl dt3         ; jump if more digits follow
        lda #0          ; clear digit between time and temp
        sta disbuf+4
.dt5    jmp display     ; show the time
.dt6    lda #0          ; load data to clear the display
        ldx #4
        jsr clr2        ; clear the last four digits
        jmp display     ; show the time without temperature/

.displayDate
        ldx #7          ; load counter
.dd1    ldy date,x      ; load digit from time
        lda font,y      ; load display data
        sta disbuf,x    ; store in display buffer
        dex             ; decrement x
        bpl dd1
        lda disbuf+3    ; set decimal point after year
        eor #&80
        sta disbuf+3
        lda disbuf+5    ; set decimal point after month			
        eor #&80
        sta disbuf+5
        jmp display     ; show the date

.setupdown
        lda mode        ; load display mode
        eor #&FF        ; toggle it
        sta mode        ; store display mode
        jmp loop5       ; return to main routine

.updateTime
        jsr timer       ; wait for the next second to complete
        bne break       ; if not second then jump
        jsr settimer    ; reset timer
        inc seconds     ; increment seconds
        lda seconds     ; load seconds
        cmp #60         ; minute passed?
        bne break       ; no, break out from loop
        sta temperature ; update temperature
        lda #0          ; reset seconds
        sta seconds
        sta mode        ; set display mode back to time
        inc time+3      ; incement minutes low
        lda time+3      ; load minutes low
        cmp #10         ; check for overflow
        bne break       ; jump if no overflow
        lda #0          ; reset minutes low
        sta time+3
        inc time+2      ; increment minutes high
        lda time+2      ; check for overflow
        cmp #6
        bne break       ; jump if no overflow
        lda #0          ; reset minutes high
        sta time+2      ; reset minutes high

        inc time+1      ; incement hours low
        lda time+1      ; load hours low
        cmp #4          ; check for end of the day (hour becomes 24)
        bne upd1        ; jump if not 4
        lda time        ; load hour high
        cmp #2          ; is it 2
        bne upd1        ; no, so the hour is not 24
        lda #0          ; the hour is 24, reset it to 00
        sta time
        sta time+1
        jsr updateDate  ; time to increment the date
        jmp break       ; jump to end of routine

.upd1   cmp #10         ; check for overflow
        bne break       ; jump if no overflow
        lda #0          ; reset hours low
        sta time+1
        inc time+0      ; increment hours high

.break  rts             ; return

.updateDate
        ; convert the day and month to binary values
        ldx date+6      ; load high byte of day
        lda mul10,x     ; "multiply" by 10
        clc             ; clear carry
        adc date+7      ; add low byte of day
        sta binday      ; store binary value of day
        ldx date+4      ; load high byte of month
        lda mul10,x     ; "multiply" by 10
        clc
        adc date+5      ; add low byte of month
        sta binmonth    ; store binary value of month

        lda binday      ; load day value
        cmp #28         ; compare to 28 (every month has at least 28 days)
        bmi upd28       ; jmp if less than 28
        
        lda binmonth    ; load month value
        cmp #2          ; is it February
        beq newmonth    ; yes, it's Februari so start a new month
        
        lda binday      ; reload day value
        cmp #30         ; is it the last day of April, June, September or November
        bmi upd28       ; it's less than 30 so always jump to increment the date
        bne newmonth    ; no then it's the 31th of the month so a new month must start
        lda binmonth    ; load month value
        cmp #4          ; is it April
        beq newmonth    ; yes, then jump to start a new month
        cmp #6          ; is it June
        beq newmonth    ; yes, then jump to start a new month
        cmp #9          ; is it September
        beq newmonth    ; yes, then jump to start a new month
        cmp #11         ; is it November
        beq newmonth    ; yes, then jump to start a new month
        ; It was the 30th of another month so we drop here to increment the day

.upd28  inc date+7      ; increment date low byte
        lda date+7      ; load new date low byte
        cmp #10         ; is it 10
        bne upd29       ; no, then jump
        lda #0          ; set low byte to 0
        sta date+7
        inc date+6      ; increment date high byte
.upd29  rts             ; because the date was less than 28 we can save return here

.newmonth               ; reset the day to 1 and increment the month (and year if applicable)
        lda #1          ; reset day
        sta date+7
        sta binday
        lda #0
        sta date+6
        inc binmonth    ; increment month value
        lda binmonth    ; load month value
        cmp #13         ; is this the end of the year
        beq newyear     ; yes, jump to happy new year
        inc date+5      ; increment month low byte
        lda date+5      ; load month low byte
        cmp #10         ; is it 10
        bne nm1         ; no then jump to the end of this routine
        lda #0          ; set low byte to 0
        sta date+5
        inc date+4      ; increment high byte of date
.nm1    rts             ; it's save to return here

.newyear
        lda #1          ; reset the month to 1
        sta date+5
        lda #0
        sta date+4
        inc date+3      ; increment year low byte
        lda date+3      ; load year low byte
        cmp #10         ; is it 10
        bne nm1         ; no, end this routine
        lda #0          ; reset low byte of the year
        sta date+3
        inc date+2      ; increment year high byte
        rts             ; return here, introducing a date bug by the year 2100 :-(

.setclock
        jsr clear       ; reset time and date
        lda #&FF        ; set scanmode to wait for key
        sta dismode
        jsr clear       ; clear the display
        ldx #0          ; loop four times to read the time
        stx seconds     ; initialize seconds counter
.set1   jsr display     ; display the time and wait for key
        sta time,x      ; store the key in memory
        tay
        lda font,y
        sta disbuf,x
        inx             ; increment counter/index
        cpx #4          ; test for end of time
        bne set1        ; jump if digits follow
        jsr clear       ; clear the display
        ldx #0          ; loop eight times to read the date
.set2   jsr display     ; display the date and wait for key
        sta date,x      ; store key in memory
        tay
        lda font,y
        sta disbuf,x
        inx             ; increment counter/index
        cpx #8          ; test for end of date
        bne set2        ; jump if digits follow
        lda #0          ; load A with 0 for return
        jsr clr1        ; clear the displays
        jmp loop1       ; jump to main routine

.reset  ldx #16         ; set time to 00:00 0000-00-00. Each digit is stored in its own memory location
        lda #0
.rst1   sta time,X
        dex
        bpl rst1
        rts

.clear  lda #8
.clr1   ldx #0        
.clr2   sta disbuf,x
        inx
        cpx #8
        bne clr2
        rts

.mul10  equb    00,10,20,30


.timer  lda temperature
        cmp #'T'
        beq timer3
        dec timerl
        bne timer2
        dec timerh
.timer2 lda timerl
        ora timerh
        rts
.timer3 lda hwtimer
        rts

.settimer
        pha
        lda temperature
        cmp #'T'
        beq settimer2
        lda #<timerv
        sta timerl
        lda #>timerv
        sta timerh
.settimer1
        pla
        rts
.settimer2
        lda #100
        sta hwtimer
        pla
        rts

.end

SAVE "clock.bin", begin, end
