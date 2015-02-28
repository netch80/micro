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
    a.open(None, dbtype = bdb.DB_RECNO, flags = bdb.DB_CREATE)
    cx = a.cursor()
    print("Started with an empty table")
    assertEquals(cx.set_range(1), None)
    print('Putting initial set')
    a.put(100, '2')
    a.put(300, '444')
    assertEquals(a.get(100), b'2')
    assertEquals(a.get(300), b'444')
    a.put(150, ''.join('%02x' % (x,) for x in range(0, 90)))
    a.delete(100)
    assertRaises(lambda: a.delete(100), bdb.DBKeyEmptyError)
    a.put(100, '2')
    print('Getting with cursor set*()')
    assertEquals(cx.set(100), (100, b'2'))
    t = cx.next()
    assertEquals(t[0], 150)
    t = cx.prev()
    assertEquals(t[0], 100)
    t = cx.next()
    assertEquals(t[0], 150)
    assertEquals(cx.next(), (300, b'444'))
    print('Getting with cursor get*()')
    assertEquals(cx.get(100, None, bdb.DB_SET), (100, b'2'))
    assertEquals(cx.get(101, None, bdb.DB_SET), None)
    assertEquals(cx.get(300, None, bdb.DB_SET, dlen = 1, doff = 0),
            (300, b'4'))
    assertEquals(cx.get(300, None, bdb.DB_SET, dlen = 2, doff = 1),
            (300, b'44'))
    print('Write a key partially')
    a.put(750, '000000')
    a.put(750, '22', dlen = 2, doff = 2)
    assertEquals(a.get(750), b'002200')
    assertEquals(a.get(750, dlen = 3, doff = 0), b'002')
    assertEquals(a.get(750, dlen = 3, doff = 3), b'200')
    print("Test succeeded")
