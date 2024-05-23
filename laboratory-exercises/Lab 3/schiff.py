#!/usr/bin/env python3 
# vim: set ts=4 sw=4 et:


#
#   MCI ES - exercise 3 - reference solution
#   Roland Lezuo <roland.lezuo@embedded-solutions.at> (c) 2024
#


#
# on linux you can let two instances play against each other using a virtual pair of serial ports
#
# in one terminal start the serial port loop:
#   $ socat -d -d pty,rawer,echo=0 pty,rawer,echo=0
#
# start on instace as player 2, you can use the spy:// protocol for more debugging fun
#   $ python schiff.py spy:///dev/pts/5 -2
#
# in another terminal start a player 1 instace and let them play 1000 games
#   $ python3 schiff.py spy:///dev/pts/4 -r -t2 -l1
#


#
# WARNING: some hackish code is ahead, read with care
#
   

import logging
import datetime
import random
import serial
import signal
import enum
import sys
import io
import re

# field configuration: key ... ship length, value ... nr. of such ships
nr_ships = {
    5: 1,
    4: 2,
    3: 3,
    2: 4,
}

# some constatns
FIELD_SZ = 10
IO_MAX_LEN = 42

# protocol state machine
class MainStates(enum.Enum):
    INIT = 1,
    START_S1 = 2,
    START_S2 = 3,
    FIELD = 4,
    PLAY = 5,
    RESULT = 6,
    GAMEEND = 7,
    ERROR = 8,
    UNEXPECTED = 9
    MYBAD = 10

class FireSolution:
    """a base class for all Fire Solutions

    defines get_coord / upate interface and generates a list of all candidates
    must be overloaded as get_coord asserts
    """
    def __init__(self, their_f, sz=FIELD_SZ):
        self.their_f = their_f
        self.sz = sz
        # all candiate coordiantes to shoot at
        self.cand = [(x,y) for x in range(0, sz) for y in range(0, sz)]

    def get_coord(self):
        """get next coordinates to fire at
        Returns:
            ((int,int)): tuple of row column to fire at, may raise if no more possibilities to fire at
        """
        # overload me and implement me!
        assert(False)

    def update(self, coord, was_a_hit):
        """get next coordinates to fire at
        Args:
            coord ((int,int)): tuple of row column where we fired
            was_a_hit (bool): whether we hit there of not
        Returns:
            None
        """
        # we ignore this information in the base class
        pass
    

class StupidFireSolution(FireSolution):
    """a very basic fire solution, a truly random player

    should be slightly better than just stupid, is a good baseline for a tournament
    """
    def get_coord(self):
        if len(self.cand) == 0:
            raise IndexError('no more fire coords, the enemy MUST be dead already, liar!')
        # pick a random candiate (and remove form candidate list)
        return self.cand.pop(random.randint(0, len(self.cand)-1))

class SmartFireSolution(StupidFireSolution):
    """a moderatly smart fire solution, if it has hit a ship it will try to sink it

    does not exploit information it gains from the protocol yet (which would be cheating)
    may be improved to not fire at certain fields at all (i.e. those surrounding sunk ships)
    """
    DIR_UNKNOWN = 0
    DIR_HORIZ = 1
    DIR_VERT = 2

    def __init__(self, their_f, sz=FIELD_SZ):
        super().__init__(their_f, sz)
        self.direction = self.DIR_UNKNOWN
        self.was_a_hit = False

    def get_coord(self):
        if not self.was_a_hit:
            return super().get_coord()
        else:
            if self.direction == self.DIR_UNKNOWN:
                if len(self.hit_cand) == 0:
                    logging.debug("XXX hit_cand empty")
                    self.was_a_hit = False
                    return super().get_coord()
                coord = self.hit_cand.pop(random.randint(0, len(self.hit_cand)-1))
                try:
                    self.cand.remove(coord)
                except:
                    # may not be in list any more ... it's okay
                    pass
                return coord
            else:
                x,y = self.hit_coord
                if self.direction == self.DIR_VERT:
                    y1,y2 = self.hit_cand
                    assert(y1 != -1 or y2 != self.sz)
                    if y1 != -1:
                        coord = (x, y1)
                        if coord in self.cand: self.cand.remove(coord)
                        return coord
                    else:
                        coord = (x, y2)
                        if coord in self.cand: self.cand.remove(coord)
                        return coord
                elif self.direction == self.DIR_HORIZ:
                    x1,x2 = self.hit_cand
                    assert(x1 != -1 or x2 != self.sz)
                    if x1 != -1:
                        coord = (x1, y)
                        if coord in self.cand: self.cand.remove(coord)
                        return coord
                    else:
                        coord = (x2, y)
                        if coord in self.cand: self.cand.remove(coord)
                        return coord
                else:
                    assert(False)
 


    def update(self, coord, was_a_hit):
        if was_a_hit:
            if not self.was_a_hit:
                # initial hit on that ship
                self.direction = self.DIR_UNKNOWN
                self.hit_coord = coord
                self.hit_cand = []
                x,y = coord
                if x > 0: self.hit_cand.append((x-1,y))
                if x < self.sz-1: self.hit_cand.append((x+1,y))
                if y > 0: self.hit_cand.append((x,y-1))
                if y < self.sz-1: self.hit_cand.append((x,y+1))

                for c in self.hit_cand:
                    if not c in self.cand:
                        self.hit_cand.remove(c)
                if len(self.hit_cand) == 0:
                    logging.debug("XXX hit w/o further hit cand")
                    self.was_a_hit = False
                logging.debug("XXX initial hit cand {}".format(self.hit_cand))
                self.was_a_hit = True
            else:
                if self.direction == self.DIR_UNKNOWN:
                    # second hit on ship, we now determine the direction
                    xo,yo = self.hit_coord
                    x,y = coord
                    if x == xo:
                        self.direction = self.DIR_VERT
                    if y == yo:
                        self.direction = self.DIR_HORIZ
                    if self.direction == self.DIR_UNKNOWN:
                        logging.debug("XXX 2nd shot no direction {} -> {}?".format(self.hit_coord, coord))
                        self.was_a_hit = False
                    else:
                        if self.direction == self.DIR_VERT:
                            y1 = min(y,yo)
                            y2 = max(y,yo)
                            if abs(y1-y2) != 1:
                                logging.debug("XXX 2nd hit offsets?")
                                self.was_a_hit = False
                            self.hit_cand =(y1-1, y2+1) # left and right of two hit ship parts
                        else:             
                            x1 = min(x,xo)
                            x2 = max(x,xo)
                            if abs(x1-x2) != 1:
                                logging.debug("XXX 2nd hit offsets?")
                                self.was_a_hit = False
                            self.hit_cand = (x1-1, x2+1)   # above and below two hit ship parts
                    logging.debug("XXX directed hit cand {}".format(self.hit_cand))
                else:
                    x,y = coord
                    if self.direction == self.DIR_VERT:
                        y1,y2 = self.hit_cand
                        if y == y2:
                            y2 += 1
                        elif y == y1:
                            y1 -= 1
                        else:
                            logging.debug("XXX can not grow cand in vert")
                            assert(False)
                        self.hit_cand = (y1,y2)
                    elif self.direction == self.DIR_HORIZ: 
                        x1,x2 = self.hit_cand
                        if x == x2:
                            x2 += 1
                        elif x == x1:
                            x1 -= 1
                        else:
                            logging.debug("XXX can not grow cand in hori")
                            assert(False)
                        self.hit_cand = (x1,x2)
                    else:
                        assert(False)
        else:   # we did not hit last round
            if self.was_a_hit:
                if self.direction == self.DIR_UNKNOWN:
                    #nothing to do
                    pass
                else:
                    logging.debug("XXX end of ship {} ({})".format(coord, self.hit_cand))
                    x,y = coord
                    if self.direction == self.DIR_VERT:
                        y1,y2 = self.hit_cand
                        if y == y1:
                            y1 = -1
                        elif y == y2:
                            y2 = self.sz
                        else:
                            assert(False)
                        self.hit_cand = (y1,y2)
                    elif self.direction == self.DIR_HORIZ:
                        x1,x2 = self.hit_cand
                        if x == x1:
                            x1 = -1
                        elif x == x2:
                            x2 = self.sz
                        else:
                            assert(False)
                        self.hit_cand = (x1,x2)
                    else:
                        assert(False)

                    if self.hit_cand == (-1,self.sz):
                        logging.debug("XXX we sunk something!")
                        self.was_a_hit = False
                    else:
                        logging.debug("XXX still onto the ship {}".format(self.hit_cand))
            else:
                pass    #nothing to do


# the game field
class Field:
    """our game field

    will generate a truely random field for each game
    detects the "we lose" condition
    generates pre-formatted F and R records
    """
    def __init__(self, sz=FIELD_SZ):
        # the placing algoithm may get stuck because ships may be placed in a way preventing all shipds from beeing placed
        # we thus try a few iterations and give up eventually starting from scratch
        # one could be more clever here, e.g. implementing a backtracking solution
        self.MAX_RETRIES = 1000
        generated = False
        while not generated:
            logging.debug("generating field")
            generated = self.generate_field(sz)

    # returns True when all ships have been placed, False if had to give up
    def generate_field(self, sz):
        while True:
            retry_counter = 0
            self.f = dict()
            self.sz = FIELD_SZ
            for x in range(0, self.sz):
                for y in range(0, self.sz):
                    self.f[self.xy_to_idx(x, y)] = 0


            for k,v in nr_ships.items():
                for i in range(0, v):
                    in_conflict = True
                    while in_conflict:
                        d = random.choice(['vert', 'horiz'])

                        if d == 'vert':
                            x = random.randint(0, sz-k-1)
                            y = random.randint(0, sz-1)
                        else:
                            x = random.randint(0, sz-1)
                            y = random.randint(0, sz-k-1)

                        logging.debug("trying to place a ship of length {} in {} direction at {},{}".format(k, d, x, y))
                        in_conflict = any(map(lambda xy: self.f[self.xy_to_idx(xy[0],xy[1])] != 0, self.surr_fields(x, y, d, k)))
                        if in_conflict:
                            logging.debug("placing ship of length {} in {} direction at {},{} failed".format(k, d, x, y))
                            retry_counter += 1
                            if retry_counter >= self.MAX_RETRIES:
                                # evntually bail out
                                return False
                        else:
                            logging.debug("placed a ship if length {} at {},{}".format(k, x, y))
                            if d == 'vert':
                                for kd in range(0, k):
                                    self.f[self.xy_to_idx(x+kd, y)] = k
                            else:
                                for kh in range(0, k):
                                    self.f[self.xy_to_idx(x, y+kh)] = k

            # pre-calcuate the sf-records (we do not cheat)
            self.sf_records = []
            for x in range(0, self.sz):
                f = "SF{}D".format(x)
                for y in range(0, self.sz):
                    f += "{}".format(self.f[self.xy_to_idx(x, y)])
                self.sf_records.append(f)

            # all ships have been placed
            return True

    # x ... horizontal, y ... vertical direction
    def xy_to_idx(self, x, y):
        return x*self.sz+y

    def shot_at(self, x, y):
        if self.f[self.xy_to_idx(x,y)] in [0, 'W']:
            self.f[self.xy_to_idx(x,y)] = 'W'
            return False
        else:
            self.f[self.xy_to_idx(x,y)] = 'T'
            return True

    def ships_left(self):
        return sum(map(lambda x: 0 if x in [0, 'W', 'T'] else 1, self.f.values()))


    def __str__(self):
        s = "Field:\n"
        for x in range(0, self.sz):
            s += "    "
            for y in range(0, self.sz):
                s += "{}".format(self.f[self.xy_to_idx(x,y)])
            s += '\n'
        return s

    def get_cs_record(self):
        r ="" 
        for x in range(0, self.sz):
            r += "{}".format(sum(map(lambda y: self.f[self.xy_to_idx(x,y)] != 0, range(0, self.sz))))
        return r

    def get_sf_records(self):
        return self.sf_records

    # returns index of all surrounding fields on map for ship of length k in direction d starting at x,y
    def surr_fields(self, x, y, d, k):
        s = []
        if d == 'vert':
            for yy in [y-1, y, y+1]:
                if yy < 0: continue
                if yy >= self.sz: continue

                for xx in range(x-1, x+k+1):
                    if xx < 0: continue
                    if xx >= self.sz: continue
                    s.append( (xx, yy) )
        else:
             for xx in [x-1, x, x+1]:
                if xx < 0: continue
                if xx >= self.sz: continue

                for yy in range(y-1, y+k+1):
                    if yy < 0: continue
                    if yy >= self.sz: continue
                    s.append( (xx, yy) )
        return s


#
# functions for communication
#

def io_read(sio):
    """received a message and parses it, eats all comments received
    Args:
        sio (serial.Serial): serial communication object for I/O
    Returns:
        dict with message-type specific key, 'R' encodes message type
    """

    l = ""
    while True:
        while True:
            c = sio.read(1)
            if c == b"":
                # timeout
                return { 'R': 'ERROR', 'text': 'timeout' }

            l += c.decode('ascii')

            if c == b'\n':
                break

        logging.info("received: {}".format(l))
        # we handle comments here
        if l[0] == '#':
            logging.info("they say: {}".format(l[1:-1]))
            continue
        else:
            break

    if l.startswith('START'):
        return { 'R': 'START', 'player': l[5:-1] }
    elif l.startswith('CS'):
        if not re.match("^CS[0-9]{10}$", l):
            return { 'R': 'ERROR', 'text': 'CS not correct' }
        return { 'R': 'CS', 'values' : [ int(x) for x in l[2:-1] ] }
    elif l[0] == 'T' or l[0] == 'W':
        return { 'R': 'HM', 'hm': l[0] }
    elif l.startswith('BOOM'):
        if not re.match("^BOOM[0-9][0-9]$", l):
            return { 'R': 'ERROR', 'text': 'BOOM not correct' }
        try:
            x = int(l[4])
            y = int(l[5])
        except:
            return { 'R': 'ERROR', 'text': 'BOOM not an int' }
        if x<0 or x>=10:
            return { 'R': 'ERROR', 'text': 'BOOM x out of range' }
        if y<0 or y>10:
            return { 'R': 'ERROR', 'text': 'BOOM y out of range' }
        return { 'R': 'BOOM', 'x': x, 'y': y }
    elif l.startswith('SF'):
        if not re.match("^SF[0-9]D[0-5]{10}$", l):
            return { 'R': 'ERROR', 'text': 'SF not correct' }
        try:
            row = int(l[2])
        except:
            return { 'R': 'ERROR', 'text': 'SF not int' }
        return { 'R': 'SF', 'l': row, 'r': l[4:-1] }

    else:
        logging.error("received this:")
        logging.error(l)
        return { 'R': 'ERROR', 'text': 'message not known' }



def io_start(sio):
    """send I message
    Args:
        sio (serial.Serial): serial communication object for I/O
    Returns:
        None
    """
    sio.write(b"START0042\n")

def io_x(sio, x):
    """send X message
    Args:
        sio (serial.Serial): serial communication object for I/O
        x (int): random number
    Returns:
        None
    """
    sio.write("X{}\n".format(x).encode('ascii'))

def io_e(sio, e):
    """send E message
    Args:
        sio (serial.Serial): serial communication object for I/O
        e (str): error message
    Returns:
        None
    """
    sio.write("E{}\n".format(e).encode('ascii'))

def io_ee(sio):
    """send e message
    Args:
        sio (serial.Serial): serial communication object for I/O
    Returns:
        None
    """
    sio.write("emy bad!\n".encode('ascii'))



def io_cs(sio, cs):
    """send CS message
    Args:
        sio (serial.Serial): serial communication object for I/O
        cs (list): list of FIELD_SZ pre-foramtted CS record
    Returns:
        None
    """
    sio.write("CS{}\n".format(cs).encode('ascii'))

def io_sf(sio, sfs):
    """send SF message, they are fully prepared
    Args:
        sio (serial.Serial): serial communication object for I/O
        cs (list): list of FIELD_SZ pre-foramtted CS record
    Returns:
        None
    """
    for sf in sfs:
        sio.write("{}\n".format(sf).encode('ascii'))



def io_boom(sio, coord):
    """send BOOM message
    Args:
        sio (serial.Serial): serial communication object for I/O
        coord ((int,int)): tuple of row col to shoot at
    Returns:
        None
    """
    x,y = coord
    sio.write("BOOM{}{}\n".format(x, y).encode('ascii'))

def io_hm(sio, is_hit):
    """send S message
    Args:
        sio (serial.Serial): serial communication object for I/O
        is_hit (bool): last shot was a hit or miss
    Returns:
        None
    """
    sio.write(b"T\n" if is_hit else b"W\n")


# global variable to quit program
quit_ = False

# handler for ctrl+c
def ctrl_c(sig, frame):
    global quit_
    quit_ = True

# main function, executing protocol state machine
def main(sio, is_player1, single_game, play_level, official_result):
    """
    Args:
        sio (serial.Serial): serial communication object for I/O
        single_game (bool): quit after a single game
        play_level (int): strength of fireing solution currently 0,1
        official_result (bool): write an offical result file
    Returns:
        None
    """
    # our global state
    err_st = None
    st = MainStates.INIT
    nr_game = 0
    nr_wins = 0
    nr_losses = 0
    nr_mybad = 0
    nr_error = 0
    nr_unexpected = 0
    their_f = None
    rd = None
    nr_turn = None
    fire_state = None
    their_x = None
    we_lost = None
    we_won = None
    their_r = None
    their_name = "anonymous"

    if official_result:
        single_game = False

    while True:
        # all state machinery is guarded by exceptions, if we catch something we assume its our
        # fault and give the match to the opponent
        try:
            sio.flush() # make sure all data has been sent
            if quit_:
                break
            if st == MainStates.INIT:
                nr_game += 1
                if single_game and nr_game > 1:
                    break
                if official_result and nr_game > 1000:
                    break
                f = Field() # generate a new field
                logging.info(f)
                their_f = {}    # empty dict
                their_x = None
                their_name = None
                their_r = dict()
                nr_turn = 0
                fire_state = None
                we_lost = False
                we_won = False
                if is_player1:
                    sio.reset_input_buffer()    # throw away all old stuff
                    st = MainStates.START_S1
                else:
                    st = MainStates.START_S2
            elif st == MainStates.START_S1:
                if len(their_f) == 0:
                    logging.info("--- START S1 message sent")
                    io_start(sio)

                rd = io_read(sio)
                if rd['R'] == 'ERROR' and rd['text'] == 'timeout' and not their_f:
                    # we safely ignore this one
                    continue
                if rd['R'] == 'ERROR':
                    st = MainStates.ERROR
                    continue

                if rd['R'] == 'CS' and not their_f:
                    their_f = dict(zip(range(0, len(rd['values'])), rd['values']))
                    tot = sum(their_f.values())
                    if tot != sum(map(lambda x: x[0]*x[1], nr_ships.items())):
                        rd = { 'R': 'ERROR', 'text': 'CS does not sum up'}
                        st = MainStates.ERROR
                        continue
                    # send our CS message as well
                    io_cs(sio, f.get_cs_record())
                    continue
                elif rd['R'] == 'START' and their_f:
                    their_name = rd['player']
                    logging.info("playing against {}".format(their_name))
                    st = MainStates.PLAY
                    continue
                else:
                    st = MainStates.UNEXPECTED
                    continue

            elif st == MainStates.START_S2:
                # we for a START message (or CS message)
                rd = io_read(sio)
                if rd['R'] == 'ERROR' and rd['text'] == 'timeout' and not their_name:
                    # we safely ignore this one as long as we did not see a START message yet
                    continue 
                if rd['R'] == 'ERROR':
                    st = MainStates.ERROR
                    continue

                if rd['R'] == 'CS' and their_name:
                    their_f = dict(zip(range(0, len(rd['values'])), rd['values']))
                    tot = sum(their_f.values())
                    if tot != sum(map(lambda x: x[0]*x[1], nr_ships.items())):
                        rd = { 'R': 'ERROR', 'text': 'CS does not sum up'}
                        st = MainStates.ERROR
                        continue
                    # send our START message as well and proceed to play state
                    io_start(sio)
                    st = MainStates.PLAY
                    continue
                elif rd['R'] == 'START' and not their_name:
                    their_name = rd['player']
                    logging.info("playing against {}".format(their_name))

                    # now send our CS message
                    io_cs(sio, f.get_cs_record())
                    continue
                else:
                    st = MainStates.UNEXPECTED
                    continue

            elif st == MainStates.PLAY:
                logging.info("--- PLAY")
                if not fire_state:
                    if play_level == 0:
                        fire_state = StupidFireSolution(their_f) 
                    elif play_level == 1:
                        fire_state = SmartFireSolution(their_f)  
                    else:
                        logging.error("I am not that smart yet!")
                        assert(False)

                if (is_player1 and nr_turn % 2 == 0) or (not is_player1 and nr_turn % 2 != 0):
                    try:
                        coord = fire_state.get_coord()
                    except IndexError:
                        rd = { 'R': 'ERROR', 'text': 'out of fire coords, I call you a liar!'}
                        st = MainStates.ERROR
                        continue
                    io_boom(sio, coord)
                    rd = io_read(sio)
                    if rd['R'] == 'ERROR':
                        st = MainStates.ERROR
                        continue
                    if rd['R'] == 'SF':
                        logging.info("Hooray, we won!")
                        we_won = True
                        their_r[rd['l']] = rd['r']
                        for i in range(0, FIELD_SZ-1):
                            rd = io_read(sio)
                            if rd['R'] == 'ERROR':
                                st = MainStates.ERROR
                                break
                            if rd['R'] != 'SF':
                                st = MainStates.UNEXPECTED
                                break
                            if rd['l'] in their_r:
                                rd = { 'R': 'ERROR', 'text': 'SF duplicate'}
                                st = MainStates.ERROR
                                break
                            their_r[rd['l']] = rd['r']
                        if len(their_r) == FIELD_SZ:
                            st = MainStates.RESULT
                            continue

                    if rd['R'] != 'HM':
                        st = MainStates.UNEXPECTED
                        continue
                    we_hit = rd['hm'] == 'T'
                    fire_state.update(coord, we_hit)
                    if we_hit:
                        logging.info("we hit at {},{}".format(*coord))
                    else:
                        logging.info("we miss at {},{}".format(*coord))
                    nr_turn += 1
                else:
                    rd = io_read(sio)
                    if rd['R'] == 'ERROR':
                        st = MainStates.ERROR
                        continue
                    if rd['R'] != 'BOOM':
                        st = MainStates.UNEXPECTED
                        continue
                    x = rd['x']
                    y = rd['y']

                    is_hit = f.shot_at(x,y)
                    if is_hit:
                        we_lost = (f.ships_left() == 0)

                    if not we_lost:
                        io_hm(sio, is_hit)
                        logging.info("they {} at {},{}".format("hit" if is_hit else "miss", x, y))
                    else:
                        st = MainStates.RESULT
                        logging.info("we just lost!")

                    nr_turn += 1

            elif st == MainStates.RESULT:
                logging.info("--- RESULT")
                assert(we_won or we_lost)  # one of those must be true now ;)
                io_sf(sio, f.get_sf_records())
                if not we_won:
                    # if we've won, we've received their_r already, in this case receive it
                    for i in range(0, FIELD_SZ):
                        rd = io_read(sio)
                        if rd['R'] == 'ERROR':
                            st = MainStates.ERROR
                            break
                        if rd['R'] != 'SF':
                            st = MainStates.UNEXPECTED
                            break
                        if rd['l'] in their_r:
                            rd = { 'R': 'ERROR', 'text': 'SF line duplicate'}
                            st = MainStates.ERROR
                            break
                        their_r[rd['l']] = rd['r']
                    for i in range(0, FIELD_SZ):
                        if not i in their_r.keys():
                            rd = { 'R': 'ERROR', 'text': 'SF lines not correct'}
                            st = MainStates.ERROR
                            break

                # print the state of our field and their field
                if we_won:
                    nr_wins += 1
                    logging.info("OURS")
                    logging.info(f)
                    logging.info("------")
                    for i in range(0, FIELD_SZ):
                        logging.info(their_r[i])
                else:
                    nr_losses += 1
                    logging.info("THEIRS")
                    for i in range(0, FIELD_SZ):
                        logging.info(their_r[i][1:])
                    logging.info("------")
                    logging.info(f)
                    logging.debug(fire_state.cand)

                # ready for a new game
                st = MainStates.GAMEEND
                continue

            elif st == MainStates.GAMEEND:
                logging.info("--- GAMEEND")
                st = MainStates.INIT
         
            elif st == MainStates.ERROR:
                nr_error += 1
                logging.error("*** protocol error: {}".format(rd['text']))
                io_e(sio, rd['text'])
                st = MainStates.INIT
            elif st == MainStates.UNEXPECTED:
                nr_unexpected += 1
                logging.error("*** protocol unexpected message of type {}".format(rd['R']))
                io_e(sio, "unexpected message of type {}".format(rd['R']))
                st = MainStates.INIT
            elif st == MainStates.MYBAD:
                nr_mybad += 1
                io_ee(sio)
                st = MainStates.INIT
            else:
                logging.error("!!! internal error in main state machine")
                break
        except Exception as e:
            logging.critical(e, exc_info=True)
            st = MainStates.MYBAD
            continue

    print("*** main loop termination ({} wins out of {} games)".format(nr_wins, nr_game-1))
    result_fname = "res_{}.csv".format(their_name)
    with open(result_fname, "a") as f:
        if f.tell() == 0:
            f.write("# schiff.py generated official result file\n")
            f.write("Name,date played, games played, my wins, my losses, my bads, your errors, your unexpected, my win ratio\n")
        f.write("{}, {}, {}, {}, {}, {}, {}, {}, {:2f}\n".format(their_name, datetime.datetime.now().isoformat(),
            nr_game, nr_wins, nr_losses, nr_mybad, nr_error, nr_unexpected, nr_wins/nr_game))



# argument handling and setup code
if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: {} /dev/serial [-2] [-s] [-tx] [-lx] [-r]".format(sys.argv[0]))
        print("   -2 ... play as player2, otherwise we are player1")
        print("   -s ... play a single game only")
        print("   -t ... trace level")
        print("   -l ... level of smartness")
        print("   -r ... record an official result, i.e. 500 games")
        sys.exit(1)
    ser = sys.argv[1]
    opt_args = sys.argv[2:]

    signal.signal(signal.SIGINT, ctrl_c)

    is_player1 = True
    single_game = False
    official_result = False
    trace_level = 0
    play_level = 0
    while len(opt_args):
        a = opt_args.pop()
        if a == "-s":
            single_game = True
        elif a == "-2":
            is_player1 = False
        elif a == "-r":
            official_result = True
        elif len(a) == 3 and a[1] == 'l':
            play_level = int(a[2])
        elif len(a) == 3 and a[1] == 't':
            trace_level = int(a[2])
        else:
            print("argument {} ignored".format(a))

    if trace_level == 0: 
        trace_level = 2 # INFO
    logging.basicConfig(level=trace_level*10)   # c.f. python default logging levels

    s = serial.serial_for_url(ser, 9600, timeout=2)
    main(s, is_player1, single_game, play_level, official_result)
