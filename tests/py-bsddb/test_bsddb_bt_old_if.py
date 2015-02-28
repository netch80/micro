#!/usr/bin/env python3

import bsddb3

def assertEquals(e, f, pn = False, expl = None):
    if e != f:
        msg = 'expected: %r; found: %r' % (x, y,)
        if expl:
            msg += ': ' + expl
        raise AssertionError(msg)
    if pn:
        if expl:
            print(expl, ': ', f, sep = '')
        else:
            print(f)

def assertRaises(call, expl = None):
    try:
        call()
    except Exception:
        pass
    else:
        msg = 'exception expected for: %r' % (call,)
        if expl:
            msg += ': ' + expl
        raise AssertionError(msg)

if __name__ == "__main__":
    a = bsddb3.btopen(None)
    print("Initial filling")
    a[b'1'] = '2'
    a[b'3'] = '444'
    assertEquals(a.get(b'1'), b'2')
    assertEquals(a.get(b'3'), b'444')
    a[b'15'] = '23'
    print('Setting location to 1')
    assertEquals(a.set_location(b'1'), (b'1', b'2'))
    assertEquals(a.next(), (b'15', b'23'))
    assertEquals(a.next(), (b'3', b'444'))
    print('Setting location to 11')
    assertEquals(a.set_location(b'11'), (b'15', b'23'))
    assertEquals(a.next(), (b'3', b'444'))
    assertRaises(lambda: a.next())
    print("Test succeeded")
