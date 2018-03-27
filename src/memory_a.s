    .globl	memset
memset:
*    move.w  14(%sp),%d0         | d0 = len
    move.w  10(%sp),%d0
    jeq     .L02

    move.l  4(%sp),%a0          | a0 = to
*    move.b  11(%sp),%d1         | d1 = value
    move.b  9(%sp),%d1

    cmpi.w  #15,%d0             | len < 16 ?
    jhi     .L30

    subq.w  #1,%d0

.L01:
    move.b  %d1,(%a0)+          | normal copy
    dbra    %d0,.L01

.L02:
    rts

.L30:
    move.w  %d2,-(%sp)

    move.b  %d1,%d2
    lsl.w   #8,%d1
    move.b  %d2,%d1
    move.w  %d1,%d2
    swap    %d1
    move.w  %d2,%d1             | d1 = value | (value << 8) | (value << 16) | (value << 24)

    move.w  %a0,%d2
    btst    #0,%d2              | dst & 1 ? (byte aligned)
    jeq     .L34

    move.b  %d1,(%a0)+          | align to word
    subq.w  #1,%d0

.L34:
    move.w  %d0,%d2
    lsr.w   #5,%d2              | d2 = len / 32
    jeq     .L47

    subq.w #1,%d2

.L37:
    move.l  %d1,(%a0)+          | fast set
    move.l  %d1,(%a0)+
    move.l  %d1,(%a0)+
    move.l  %d1,(%a0)+
    move.l  %d1,(%a0)+
    move.l  %d1,(%a0)+
    move.l  %d1,(%a0)+
    move.l  %d1,(%a0)+
    dbra    %d2,.L37

.L47:
    move.w  %d0,%d2
    lsr.w   #2,%d2
    andi.w  #7,%d2              | d2 = (len >> 2) & 7
    jeq     .L49

    subq.w  #1,%d2

.L40:
    move.l  %d1,(%a0)+
    dbra    %d2,.L40

.L49:
    andi.w  #3,%d0              | d0 = len & 3
    jeq     .L29

    subq.w  #1,%d0

.L43:
    move.b  %d1,(%a0)+
    dbra    %d0,.L43

.L29:
    move.w  (%sp)+,%d2
    rts


    .globl	memcpy
memcpy:
*    move.w  14(%sp),%d0         | d0 = len
    move.w  12(%sp),%d0
    jeq     .L82

    move.l  4(%sp),%a1          | a1 = dst
    move.l  8(%sp),%a0          | a0 = src

    cmpi.w  #15,%d0             | len < 16 ?
    jhi     .L80

    subq.w  #1,%d0

.L83:
    move.b  (%a0)+,(%a1)+
    dbra    %d0,.L83

.L82:
    rts

.L80:
    move.w  %d2,-(%sp)

    move.w  %a0,%d1
    move.w  %a1,%d2
    eor.w   %d1,%d2
    btst    #0,%d2              | same byte alignment on src and dst ?
    jeq     .L84                | go to word copy

    move.w  %d0,%d2
    lsr.w   #3,%d2              | d2 = len >> 3
    jeq     .L104

    subq.w  #1,%d2

.L87:
    move.b  (%a0)+,(%a1)+       | fast byte copy
    move.b  (%a0)+,(%a1)+
    move.b  (%a0)+,(%a1)+
    move.b  (%a0)+,(%a1)+
    move.b  (%a0)+,(%a1)+
    move.b  (%a0)+,(%a1)+
    move.b  (%a0)+,(%a1)+
    move.b  (%a0)+,(%a1)+
    dbra    %d2,.L87

.L104:
    andi.w  #7,%d0              | d0 = len & 7
    jeq     .L89

    subq.w  #1,%d0

.L90:
    move.b  (%a0)+,(%a1)+
    dbra    %d0,.L90

.L89:
    move.w  (%sp)+,%d2
    rts

.L84:
    btst    #0,%d1              | src/dst address byte aligned ?
    jeq     .L91

    move.b  (%a0)+,(%a1)+       | align to word
    subq.w  #1,%d0

.L91:
    move.w  %d0,%d2
    lsr.w   #5,%d2              | d2 = len / 32
    jeq     .L108

    subq.w  #1,%d2

.L94:
    move.l  (%a0)+,(%a1)+       | fast copy
    move.l  (%a0)+,(%a1)+
    move.l  (%a0)+,(%a1)+
    move.l  (%a0)+,(%a1)+
    move.l  (%a0)+,(%a1)+
    move.l  (%a0)+,(%a1)+
    move.l  (%a0)+,(%a1)+
    move.l  (%a0)+,(%a1)+
    dbra    %d2,.L94

.L108:
    move.w  %d0,%d2
    lsr.w   #2,%d2
    andi.w  #7,%d2              | d2 = (len >> 2) & 7
    jeq     .L110

    subq.w  #1,%d2

.L97:
    move.l  (%a0)+,(%a1)+
    dbra    %d2,.L97

.L110:
    andi.w  #3,%d0              | d0 = len & 3
    jeq     .L79

    subq.w  #1,%d0

.L100:
    move.b  (%a0)+,(%a1)+
    dbra    %d0,.L100

.L79:
    move.w  (%sp)+,%d2
    rts
