#
# This file is part of the xTuple ERP: PostBooks Edition, a free and
# open source Enterprise Resource Planning software suite,
# Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
# It is licensed to you under the Common Public Attribution License
# version 1.0, the full text of which (including xTuple-specific Exhibits)
# is available at www.xtuple.com/CPAL.  By using this software, you agree
# to be bound by its terms.
#
TEMPLATE = subdirs
SUBDIRS = common \
          scriptapi \
          widgets/dll.pro \
          widgets \
          guiclient

CONFIG += ordered

TRANSLATIONS = share/dict/xTuple.ar_eg.ts \
               share/dict/xTuple.base.ts \
               share/dict/xTuple.bg.ts \
               share/dict/xTuple.cs.ts \
               share/dict/xTuple.de_at.ts \
               share/dict/xTuple.de_ch.ts \
               share/dict/xTuple.de.ts \
               share/dict/xTuple.en_ca.ts \
               share/dict/xTuple.es_ar.ts \
               share/dict/xTuple.es_mx.ts \
               share/dict/xTuple.es.ts \
               share/dict/xTuple.et_ee.ts \
               share/dict/xTuple.fr_ca.ts \
               share/dict/xTuple.fr.ts \
               share/dict/xTuple.hr.ts \
               share/dict/xTuple.it.ts \
               share/dict/xTuple.ja.ts \
               share/dict/xTuple.nl.ts \
               share/dict/xTuple.no.ts \
               share/dict/xTuple.pl.ts \
               share/dict/xTuple.pt_br.ts \
               share/dict/xTuple.pt.ts \
               share/dict/xTuple.ru.ts \
               share/dict/xTuple.sk.ts \
               share/dict/xTuple.tr.ts \
               share/dict/xTuple.uk.ts \
               share/dict/xTuple.zh_hk.ts \
               share/dict/xTuple.zh.ts \
               share/dict/xTuple.zh_tw.ts

INSTALLS = certificates dictionaries translations

DESTDIR=bin

macx {
  EXTRASDIR=$$absolute_path($${DESTDIR})/xtuple.app/Contents/Resources
} else {
  EXTRASDIR=$$absolute_path($${DESTDIR})
}

certificates.path = $$absolute_path($${EXTRASDIR})/certificates
certificates.files = share/certificates/*

dictionaries.path = $$absolute_path($${EXTRASDIR})/hunspell
dictionaries.files = hunspell/*.aff hunspell/*.dic

translations.path = $$absolute_path($${EXTRASDIR})/dict
translations.files = $$replace(TRANSLATIONS, ts, qm)
translations.extra = cd share/dict && $$dirname(QMAKE_QMAKE)/lrelease xTuple*.ts
