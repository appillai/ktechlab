/*
    KTechLab, and IDE for electronics
    Copyright (C) 2010 Zoltan Padrah

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/

#include "testqprocesswitharguments.h"

#include "qprocesswitharguments.h"

#include <QtTest/QtTest>

void TestQProcessWithArguments::launchLs()
{
    QProcessWithArguments p;
    p << "ls";
    p << "-la";
    p.start();
    p.waitForFinished();
    Q_ASSERT( p.exitStatus() == QProcess::NormalExit );
    Q_ASSERT( p.readAllStandardOutput().length() > 0 );
}

QTEST_MAIN(TestQProcessWithArguments)
// #include "testqprocesswitharguments.moc"
