#!/usr/bin/env python3

import bsddb3.db as bdb

def assertEquals(f, e, pn = False, expl = None):
    if e != f:
        msg = 'expected: %r; found: %r' % (e, f,)
        if expl:
            msg += ': ' + expl
        raise AssertionError(msg)
    if pn:
        if expl:
            print(expl, ': ', f, sep = '')
        else:
            print(f)

def assertRaises(call, excc = Exception, expl = None):
    try:
        call()
    except excc:
        pass
    else:
        msg = 'exception expected for: %r' % (call,)
        if expl:
            msg += ': ' + expl
        raise AssertionError(msg)

if __name__ == "__main__":
    a = bdb.DB()
    ## Temporary one
    a.open(None, dbtype = bdb.DB_BTREE, flags = bdb.DB_CREATE)
    cx = a.cursor()
    print("Started with an empty table")
    assertEquals(cx.set_range(b''), None)
    print('Putting initial set')
    a.put(b'1', '2')
    a.put(b'3', '444')
    assertEquals(cx.set_range(b''), (b'1', b'2'))
    assertEquals(a.get(b'1'), b'2')
    assertEquals(a.get(b'3'), b'444')
    a.put(b'15', ''.join('%02x' % (x,) for x in range(0, 90)))
    a.delete(b'1')
    assertRaises(lambda: a.delete(b'1'), bdb.DBNotFoundError)
    a.put(b'1', '2')
    print('Getting with cursor set*()')
    assertEquals(cx.set(b'1'), (b'1', b'2'))
    t = cx.set_range(b'10')
    assertEquals(len(t), 2, expl = 't={!r}'.format(t))
    assertEquals(t[0], b'15', expl = 't={!r}'.format(t))
    assertEquals(cx.next(), (b'3', b'444'))
    print('Getting with cursor get*()')
    assertEquals(cx.get(b'1', None, bdb.DB_SET), (b'1', b'2'))
    assertEquals(cx.get(b'101', None, bdb.DB_SET), None)
    t = cx.get(b'11', None, bdb.DB_SET_RANGE, dlen = 40, doff = 0)
    assertEquals(t, (b'15', b'000102030405060708090a0b0c0d0e0f10111213'))
    t = cx.get(b'11', None, bdb.DB_SET_RANGE, dlen = 40, doff = 40)
    assertEquals(t, (b'15', b'1415161718191a1b1c1d1e1f2021222324252627'))
    print('Write a key partially')
    a.put(b'k', '000000')
    a.put(b'k', '22', dlen = 2, doff = 2)
    assertEquals(a.get(b'k'), b'002200')
    print("Test succeeded")
