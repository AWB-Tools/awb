# *****************************************************************
# *                                                               *
# Copyright (C) 2003-2008 Intel Corporation
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
# 

##
## SPEC CPU2000 benchmark configuration
##

package CPU2000_data;

use strict;

our %isFP = (
    '164.gzip' => 0,
    '175.vpr' => 0,
    '176.gcc' => 0,
    '181.mcf' => 0,
    '186.crafty' => 0,
    '197.parser' => 0,
    '252.eon' => 0,
    '253.perlbmk' => 0,
    '254.gap' => 0,
    '255.vortex' => 0,
    '256.bzip2' => 0,
    '300.twolf' => 0,
    '168.wupwise' => 1,
    '171.swim' => 1,
    '172.mgrid' => 1,
    '173.applu' => 1,
    '177.mesa' => 1,
    '178.galgel' => 1,
    '179.art' => 1,
    '183.equake' => 1,
    '187.facerec' => 1,
    '188.ammp' => 1,
    '189.lucas' => 1,
    '191.fma3d' => 1,
    '200.sixtrack' => 1,
    '301.apsi' => 1,
);

our %spec2k;

$spec2k{test} = {
    '164.gzip' => {
        'program' => 'gzip',
        'args' => 'input.compressed 2',
        'input' => '',
        'output' => 'input.compressed.out',
        'errout' => 'input.compressed.err',
        'compare' => [
            'specdiff -l 10 data/test/output/input.compressed.out input.compressed.out',
        ],
    },
    '175.vpr_route_log' => {
        'program' => 'vpr',
        'args' => 'net.in arch.in place.in route.out -nodisp -route_only -route_chan_width 15 -pres_fac_mult 2 -acc_fac 1 -first_iter_pres_fac 4 -initial_pres_fac 8',
        'input' => '',
        'output' => 'route_log.out',
        'errout' => 'route_log.err',
        'compare' => [
            'specdiff -l 10 -r 0.1 data/test/output/costs.out costs.out',
            'specdiff -l 10 -r 0.015 data/test/output/route.out route.out',
            'specdiff -l 10 -r 0.015 data/test/output/route_log.out route_log.out',
        ],
    },
    '175.vpr_place_log' => {
        'program' => 'vpr',
        'args' => 'net.in arch.in place.out dum.out -nodisp -place_only -init_t 5 -exit_t 0.005 -alpha_t 0.9412 -inner_num 2',
        'input' => '',
        'output' => 'place_log.out',
        'errout' => 'place_log.err',
        'compare' => [
            'specdiff -l 10 -r 0.015 data/test/output/place_log.out place_log.out',
        ],
    },
    '176.gcc' => {
        'program' => 'cc1',
        'args' => 'cccp.i -o cccp.s',
        'input' => '',
        'output' => 'cccp.out',
        'errout' => 'cccp.err',
        'compare' => [
            'specdiff -l 10 data/test/output/cccp.s cccp.s',
        ],
    },
    '181.mcf' => {
        'program' => 'mcf',
        'args' => 'inp.in',
        'input' => '',
        'output' => 'inp.out',
        'errout' => 'inp.err',
        'compare' => [
            'specdiff -l 10 data/test/output/inp.out inp.out',
            'specdiff -l 10 data/test/output/mcf.out mcf.out',
        ],
    },
    '186.crafty' => {
        'program' => 'crafty',
        'args' => '',
        'input' => 'crafty.in',
        'output' => 'crafty.out',
        'errout' => 'crafty.err',
        'compare' => [
            'specdiff -l 10 data/test/output/crafty.out crafty.out',
        ],
    },
    '197.parser' => {
        'program' => 'parser',
        'args' => '2.1.dict -batch',
        'input' => 'test.in',
        'output' => 'test.out',
        'errout' => 'test.err',
        'compare' => [
            'specdiff -l 10 data/test/output/test.out test.out',
        ],
    },
    '252.eon_cook_log' => {
        'program' => 'eon',
        'args' => 'chair.control.cook chair.camera chair.surfaces chair.cook.ppm ppm pixels_out.cook',
        'input' => '',
        'output' => 'cook_log.out',
        'errout' => 'cook_log.err',
        'compare' => [
            'specdiff -l 10 -a 0.005 data/test/output/pixels_out.cook pixels_out.cook',
        ],
    },
    '252.eon_kajiya_log' => {
        'program' => 'eon',
        'args' => 'chair.control.kajiya chair.camera chair.surfaces chair.kajiya.ppm ppm pixels_out.kajiya',
        'input' => '',
        'output' => 'kajiya_log.out',
        'errout' => 'kajiya_log.err',
        'compare' => [
            'specdiff -l 10 -a 0.005 data/test/output/pixels_out.kajiya pixels_out.kajiya',
        ],
    },
    '252.eon_rushmeier_log' => {
        'program' => 'eon',
        'args' => 'chair.control.rushmeier chair.camera chair.surfaces chair.rushmeier.ppm ppm pixels_out.rushmeier',
        'input' => '',
        'output' => 'rushmeier_log.out',
        'errout' => 'rushmeier_log.err',
        'compare' => [
            'specdiff -l 10 -a 0.005 data/test/output/pixels_out.rushmeier pixels_out.rushmeier',
        ],
    },
    '253.perlbmk' => {
        'program' => 'perlbmk',
        'args' => '-I. -I./lib test.pl',
        'input' => 'test.in',
        'output' => 'test.out',
        'errout' => 'test.err',
        'compare' => [
            'specdiff -l 10 data/test/output/append.out append.out',
            'specdiff -l 10 data/test/output/arith.out arith.out',
            'specdiff -l 10 data/test/output/array.out array.out',
            'specdiff -l 10 data/test/output/auto.out auto.out',
            'specdiff -l 10 data/test/output/chop.out chop.out',
            'specdiff -l 10 data/test/output/cmdopt.out cmdopt.out',
            'specdiff -l 10 data/test/output/cmp.out cmp.out',
            'specdiff -l 10 data/test/output/cond.out cond.out',
            'specdiff -l 10 data/test/output/decl.out decl.out',
            'specdiff -l 10 data/test/output/delete.out delete.out',
            'specdiff -l 10 data/test/output/do.out do.out',
            'specdiff -l 10 data/test/output/each.out each.out',
            'specdiff -l 10 data/test/output/elsif.out elsif.out',
            'specdiff -l 10 data/test/output/eval.out eval.out',
            'specdiff -l 10 data/test/output/exp.out exp.out',
            'specdiff -l 10 data/test/output/for.out for.out',
            'specdiff -l 10 data/test/output/goto.out goto.out',
            'specdiff -l 10 data/test/output/gv.out gv.out',
            'specdiff -l 10 data/test/output/if.out if.out',
            'specdiff -l 10 data/test/output/inc.out inc.out',
            'specdiff -l 10 data/test/output/index.out index.out',
            'specdiff -l 10 data/test/output/int.out int.out',
            'specdiff -l 10 data/test/output/join.out join.out',
            'specdiff -l 10 data/test/output/lex.out lex.out',
            'specdiff -l 10 data/test/output/list.out list.out',
            'specdiff -l 10 data/test/output/local.out local.out',
            'specdiff -l 10 data/test/output/method.out method.out',
            'specdiff -l 10 data/test/output/mod.out mod.out',
            'specdiff -l 10 data/test/output/my.out my.out',
            'specdiff -l 10 data/test/output/oct.out oct.out',
            'specdiff -l 10 data/test/output/ord.out ord.out',
            'specdiff -l 10 data/test/output/package.out package.out',
            'specdiff -l 10 data/test/output/pat.out pat.out',
            'specdiff -l 10 data/test/output/print.out print.out',
            'specdiff -l 10 data/test/output/push.out push.out',
            'specdiff -l 10 data/test/output/quotemeta.out quotemeta.out',
            'specdiff -l 10 data/test/output/rand.out rand.out',
            'specdiff -l 10 data/test/output/range.out range.out',
            'specdiff -l 10 data/test/output/recurse.out recurse.out',
            'specdiff -l 10 data/test/output/redef.out redef.out',
            'specdiff -l 10 data/test/output/ref.out ref.out',
            'specdiff -l 10 data/test/output/repeat.out repeat.out',
            'specdiff -l 10 data/test/output/script.out script.out',
            'specdiff -l 10 data/test/output/sleep.out sleep.out',
            'specdiff -l 10 data/test/output/sort.out sort.out',
            'specdiff -l 10 data/test/output/split.out split.out',
            'specdiff -l 10 data/test/output/sprintf.out sprintf.out',
            'specdiff -l 10 data/test/output/study.out study.out',
            'specdiff -l 10 data/test/output/subst.out subst.out',
            'specdiff -l 10 data/test/output/substr.out substr.out',
            'specdiff -l 10 data/test/output/subval.out subval.out',
            'specdiff -l 10 data/test/output/switch.out switch.out',
            'specdiff -l 10 data/test/output/symbol.out symbol.out',
            'specdiff -l 10 data/test/output/term.out term.out',
            'specdiff -l 10 data/test/output/test.out test.out',
            'specdiff -l 10 data/test/output/time.out time.out',
            'specdiff -l 10 data/test/output/undef.out undef.out',
            'specdiff -l 10 data/test/output/unshift.out unshift.out',
            'specdiff -l 10 data/test/output/vec.out vec.out',
            'specdiff -l 10 data/test/output/while.out while.out',
        ],
    },
    '254.gap' => {
        'program' => 'gap',
        'args' => '-l ./ -q -m 64M',
        'input' => 'test.in',
        'output' => 'test.out',
        'errout' => 'test.err',
        'compare' => [
            'specdiff -l 10 -r 0.01 data/test/output/test.out test.out',
        ],
    },
    '255.vortex' => {
        'program' => 'vortex',
        'args' => 'lendian.raw',
        'input' => '',
        'output' => 'vortex.out2',
        'errout' => 'vortex.err',
        'compare' => [
            'specdiff -l 10 data/test/output/vortex.out vortex.out',
        ],
    },
    '256.bzip2' => {
        'program' => 'bzip2',
        'args' => 'input.random 2',
        'input' => '',
        'output' => 'input.random.out',
        'errout' => 'input.random.err',
        'compare' => [
            'specdiff -l 10 data/test/output/input.random.out input.random.out',
        ],
    },
    '300.twolf' => {
        'program' => 'twolf',
        'args' => 'test',
        'input' => '',
        'output' => 'test.stdout',
        'errout' => 'test.err',
        'compare' => [
            'specdiff -l 10 -o data/test/output/test.out test.out',
            'specdiff -l 10 -o data/test/output/test.pin test.pin',
            'specdiff -l 10 -o data/test/output/test.pl1 test.pl1',
            'specdiff -l 10 -o data/test/output/test.pl2 test.pl2',
            'specdiff -l 10 -o data/test/output/test.sav test.sav',
            'specdiff -l 10 -o data/test/output/test.stdout test.stdout',
            'specdiff -l 10 -o data/test/output/test.sv2 test.sv2',
            'specdiff -l 10 -o data/test/output/test.twf test.twf',
        ],
    },
    '168.wupwise' => {
        'program' => 'wupwise',
        'args' => '',
        'input' => '',
        'output' => 'wupwise.out',
        'errout' => 'wupwise.err',
        'compare' => [
            'specdiff -l 10 -r 0.005 data/test/output/te.out te.out',
            'specdiff -l 10 -r 0.001 data/test/output/wupwise.out wupwise.out',
        ],
    },
    '171.swim' => {
        'program' => 'swim',
        'args' => '',
        'input' => 'swim.in',
        'output' => 'swim.out',
        'errout' => 'swim.err',
        'compare' => [
            'specdiff -l 10 -a 1e-06 -r 0.2 data/test/output/SWIM7 SWIM7',
            'specdiff -l 10 -a 1e-06 -r 0.001 data/test/output/swim.out swim.out',
        ],
    },
    '172.mgrid' => {
        'program' => 'mgrid',
        'args' => '',
        'input' => 'mgrid.in',
        'output' => 'mgrid.out',
        'errout' => 'mgrid.err',
        'compare' => [
            'specdiff -l 10 -a 1e-12 -r 0.001 data/test/output/mgrid.out mgrid.out',
        ],
    },
    '173.applu' => {
        'program' => 'applu',
        'args' => '',
        'input' => 'applu.in',
        'output' => 'applu.out',
        'errout' => 'applu.err',
        'compare' => [
            'specdiff -l 10 -r 0.0001 data/test/output/applu.out applu.out',
        ],
    },
    '177.mesa' => {
        'program' => 'mesa',
        'args' => '-frames 10 -meshfile mesa.in -ppmfile mesa.ppm',
        'input' => '',
        'output' => '',
        'errout' => '',
        'compare' => [
            'specdiff -l 10 -a 6 -o -s 6 data/test/output/mesa.log mesa.log',
            'specdiff -l 10 -a 6 -o -s 6 data/test/output/mesa.ppm mesa.ppm',
        ],
    },
    '178.galgel' => {
        'program' => 'galgel',
        'args' => '',
        'input' => 'galgel.in',
        'output' => 'galgel.out',
        'errout' => 'galgel.err',
        'compare' => [
            'specdiff -l 10 -a 2e-08 -r 0.01 data/test/output/galgel.out galgel.out',
        ],
    },
    '179.art' => {
        'program' => 'art',
        'args' => '-scanfile c756hel.in -trainfile1 a10.img -stride 2 -startx 134 -starty 220 -endx 139 -endy 225 -objects 1',
        'input' => '',
        'output' => 'test.out',
        'errout' => 'test.err',
        'compare' => [
            'specdiff -l 10 -r 0.01 data/test/output/test.out test.out',
        ],
    },
    '183.equake' => {
        'program' => 'equake',
        'args' => '',
        'input' => 'inp.in',
        'output' => 'inp.out',
        'errout' => 'inp.err',
        'compare' => [
            'specdiff -l 10 -r 1e-05 data/test/output/inp.out inp.out',
        ],
    },
    '187.facerec' => {
        'program' => 'facerec',
        'args' => '',
        'input' => 'test.in',
        'output' => 'test.out',
        'errout' => 'test.err',
        'compare' => [
            'specdiff -l 10 -a 5 -r 0.2 -s 4 data/test/output/hops.out hops.out',
            'specdiff -l 10 -a 2e-07 -r 0.001 -s 0 data/test/output/test.out test.out',
        ],
    },
    '188.ammp' => {
        'program' => 'ammp',
        'args' => '',
        'input' => 'ammp.in',
        'output' => 'ammp.out',
        'errout' => 'ammp.err',
        'compare' => [
            'specdiff -l 10 -a 0.0001 -r 0.003 data/test/output/ammp.out ammp.out',
        ],
    },
    '189.lucas' => {
        'program' => 'lucas',
        'args' => '',
        'input' => 'lucas2.in',
        'output' => 'lucas2.out',
        'errout' => 'lucas2.err',
        'compare' => [
            'specdiff -l 10 data/test/output/lucas2.out lucas2.out',
        ],
    },
    '191.fma3d' => {
        'program' => 'fma3d',
        'args' => '',
        'input' => '',
        'output' => 'fma3d.out',
        'errout' => 'fma3d.err',
        'compare' => [
            'specdiff -l 10 -a 1e-07 -r 0.04 data/test/output/fma3d.out fma3d.out',
        ],
    },
    '200.sixtrack' => {
        'program' => 'sixtrack',
        'args' => '',
        'input' => 'inp.in',
        'output' => 'inp.out',
        'errout' => 'inp.err',
        'compare' => [
            'specdiff -l 10 -a 0.0005 -r 0.0005 data/test/output/inp.out inp.out',
        ],
    },
    '301.apsi' => {
        'program' => 'apsi',
        'args' => '',
        'input' => '',
        'output' => 'apsi.out',
        'errout' => 'apsi.err',
        'compare' => [
            'specdiff -l 10 -a 1e-07 -r 0.01 data/test/output/APO10 APO10',
            'specdiff -l 10 -a 1e-07 -r 0.01 data/test/output/APO11 APO11',
            'specdiff -l 10 -a 1e-07 -r 0.01 data/test/output/APO6 APO6',
            'specdiff -l 10 -a 1e-07 -r 0.01 data/test/output/APO8 APO8',
            'specdiff -l 10 -a 1e-07 -r 0.01 data/test/output/APV APV',
        ],
    },
};


$spec2k{train} = {
    '164.gzip' => {
        'program' => 'gzip',
        'args' => 'input.combined 32',
        'input' => '',
        'output' => 'input.combined.out',
        'errout' => 'input.combined.err',
        'compare' => [
            'specdiff -l 10 data/train/output/input.combined.out input.combined.out',
        ],
    },
    '175.vpr_route_log' => {
        'program' => 'vpr',
        'args' => 'net.in arch.in place.in route.out -nodisp -route_only -route_chan_width 15 -pres_fac_mult 2 -acc_fac 1 -first_iter_pres_fac 4 -initial_pres_fac 8',
        'input' => '',
        'output' => 'route_log.out',
        'errout' => 'route_log.err',
        'compare' => [
            'specdiff -l 10 -r 0.05 data/train/output/costs.out costs.out',
            'specdiff -l 10 -r 0.015 data/train/output/route.out route.out',
            'specdiff -l 10 -r 0.015 data/train/output/route_log.out route_log.out',
        ],
    },
    '175.vpr_place_log' => {
        'program' => 'vpr',
        'args' => 'net.in arch.in place.out dum.out -nodisp -place_only -init_t 5 -exit_t 0.005 -alpha_t 0.9412 -inner_num 2',
        'input' => '',
        'output' => 'place_log.out',
        'errout' => 'place_log.err',
        'compare' => [
            'specdiff -l 10 -r 0.015 data/train/output/place_log.out place_log.out',
        ],
    },
    '176.gcc' => {
        'program' => 'cc1',
        'args' => 'cp-decl.i -o cp-decl.s',
        'input' => '',
        'output' => 'cp-decl.out',
        'errout' => 'cp-decl.err',
        'compare' => [
            'specdiff -l 10 data/train/output/cp-decl.s cp-decl.s',
        ],
    },
    '181.mcf' => {
        'program' => 'mcf',
        'args' => 'inp.in',
        'input' => '',
        'output' => 'inp.out',
        'errout' => 'inp.err',
        'compare' => [
            'specdiff -l 10 data/train/output/inp.out inp.out',
            'specdiff -l 10 data/train/output/mcf.out mcf.out',
        ],
    },
    '186.crafty' => {
        'program' => 'crafty',
        'args' => '',
        'input' => 'crafty.in',
        'output' => 'crafty.out',
        'errout' => 'crafty.err',
        'compare' => [
            'specdiff -l 10 data/train/output/crafty.out crafty.out',
        ],
    },
    '197.parser' => {
        'program' => 'parser',
        'args' => '2.1.dict -batch',
        'input' => 'train.in',
        'output' => 'train.out',
        'errout' => 'train.err',
        'compare' => [
            'specdiff -l 10 data/train/output/train.out train.out',
        ],
    },
    '252.eon_kajiya_log' => {
        'program' => 'eon',
        'args' => 'chair.control.kajiya chair.camera chair.surfaces chair.kajiya.ppm ppm pixels_out.kajiya',
        'input' => '',
        'output' => 'kajiya_log.out',
        'errout' => 'kajiya_log.err',
        'compare' => [
            'specdiff -l 10 -a 0.005 data/train/output/pixels_out.kajiya pixels_out.kajiya',
        ],
    },
    '252.eon_cook_log' => {
        'program' => 'eon',
        'args' => 'chair.control.cook chair.camera chair.surfaces chair.cook.ppm ppm pixels_out.cook',
        'input' => '',
        'output' => 'cook_log.out',
        'errout' => 'cook_log.err',
        'compare' => [
            'specdiff -l 10 -a 0.005 data/train/output/pixels_out.cook pixels_out.cook',
        ],
    },
    '252.eon_rushmeier_log' => {
        'program' => 'eon',
        'args' => 'chair.control.rushmeier chair.camera chair.surfaces chair.rushmeier.ppm ppm pixels_out.rushmeier',
        'input' => '',
        'output' => 'rushmeier_log.out',
        'errout' => 'rushmeier_log.err',
        'compare' => [
            'specdiff -l 10 -a 0.005 data/train/output/pixels_out.rushmeier pixels_out.rushmeier',
        ],
    },
    '253.perlbmk_scrabbl' => {
        'program' => 'perlbmk',
        'args' => '-I. -I./lib scrabbl.pl',
        'input' => 'scrabbl.in',
        'output' => 'scrabbl.out',
        'errout' => 'scrabbl.err',
        'compare' => [
            'specdiff -l 10 data/train/output/scrabbl.out scrabbl.out',
        ],
    },
    '253.perlbmk_b.3' => {
        'program' => 'perlbmk',
        'args' => '-I./lib perfect.pl b 3',
        'input' => '',
        'output' => 'b.3.out',
        'errout' => 'b.3.err',
        'compare' => [
            'specdiff -l 10 data/train/output/b.3.out b.3.out',
        ],
    },
    '253.perlbmk_2.350.15.24.23.150' => {
        'program' => 'perlbmk',
        'args' => '-I./lib diffmail.pl 2 350 15 24 23 150',
        'input' => '',
        'output' => '2.350.15.24.23.150.out',
        'errout' => '2.350.15.24.23.150.err',
        'compare' => [
            'specdiff -l 10 data/train/output/2.350.15.24.23.150.out 2.350.15.24.23.150.out',
        ],
    },
    '254.gap' => {
        'program' => 'gap',
        'args' => '-l ./ -q -m 128M',
        'input' => 'train.in',
        'output' => 'train.out',
        'errout' => 'train.err',
        'compare' => [
            'specdiff -l 10 -r 0.01 data/train/output/train.out train.out',
        ],
    },
    '255.vortex' => {
        'program' => 'vortex',
        'args' => 'lendian.raw',
        'input' => '',
        'output' => 'vortex.out2',
        'errout' => 'vortex.err',
        'compare' => [
            'specdiff -l 10 data/train/output/vortex.out vortex.out',
        ],
    },
    '256.bzip2' => {
        'program' => 'bzip2',
        'args' => 'input.compressed 8',
        'input' => '',
        'output' => 'input.compressed.out',
        'errout' => 'input.compressed.err',
        'compare' => [
            'specdiff -l 10 data/train/output/input.compressed.out input.compressed.out',
        ],
    },
    '300.twolf' => {
        'program' => 'twolf',
        'args' => 'train',
        'input' => '',
        'output' => 'train.stdout',
        'errout' => 'train.err',
        'compare' => [
            'specdiff -l 10 -o data/train/output/train.out train.out',
            'specdiff -l 10 -o data/train/output/train.pin train.pin',
            'specdiff -l 10 -o data/train/output/train.pl1 train.pl1',
            'specdiff -l 10 -o data/train/output/train.pl2 train.pl2',
            'specdiff -l 10 -o data/train/output/train.sav train.sav',
            'specdiff -l 10 -o data/train/output/train.stdout train.stdout',
            'specdiff -l 10 -o data/train/output/train.sv2 train.sv2',
            'specdiff -l 10 -o data/train/output/train.twf train.twf',
        ],
    },
    '168.wupwise' => {
        'program' => 'wupwise',
        'args' => '',
        'input' => '',
        'output' => 'wupwise.out',
        'errout' => 'wupwise.err',
        'compare' => [
            'specdiff -l 10 -r 0.005 data/train/output/te.out te.out',
            'specdiff -l 10 -r 0.001 data/train/output/wupwise.out wupwise.out',
        ],
    },
    '171.swim' => {
        'program' => 'swim',
        'args' => '',
        'input' => 'swim.in',
        'output' => 'swim.out',
        'errout' => 'swim.err',
        'compare' => [
            'specdiff -l 10 -a 1e-06 -r 0.2 data/train/output/SWIM7 SWIM7',
            'specdiff -l 10 -a 1e-06 -r 0.001 data/train/output/swim.out swim.out',
        ],
    },
    '172.mgrid' => {
        'program' => 'mgrid',
        'args' => '',
        'input' => 'mgrid.in',
        'output' => 'mgrid.out',
        'errout' => 'mgrid.err',
        'compare' => [
            'specdiff -l 10 -a 1e-12 -r 0.001 data/train/output/mgrid.out mgrid.out',
        ],
    },
    '173.applu' => {
        'program' => 'applu',
        'args' => '',
        'input' => 'applu.in',
        'output' => 'applu.out',
        'errout' => 'applu.err',
        'compare' => [
            'specdiff -l 10 -r 0.0001 data/train/output/applu.out applu.out',
        ],
    },
    '177.mesa' => {
        'program' => 'mesa',
        'args' => '-frames 500 -meshfile mesa.in -ppmfile mesa.ppm',
        'input' => '',
        'output' => '',
        'errout' => '',
        'compare' => [
            'specdiff -l 10 -a 6 -o -s 6 data/train/output/mesa.log mesa.log',
            'specdiff -l 10 -a 6 -o -s 6 data/train/output/mesa.ppm mesa.ppm',
        ],
    },
    '178.galgel' => {
        'program' => 'galgel',
        'args' => '',
        'input' => 'galgel.in',
        'output' => 'galgel.out',
        'errout' => 'galgel.err',
        'compare' => [
            'specdiff -l 10 -a 2e-08 -r 0.01 data/train/output/galgel.out galgel.out',
        ],
    },
    '179.art' => {
        'program' => 'art',
        'args' => '-scanfile c756hel.in -trainfile1 a10.img -stride 2 -startx 134 -starty 220 -endx 184 -endy 240 -objects 3',
        'input' => '',
        'output' => 'train.out',
        'errout' => 'train.err',
        'compare' => [
            'specdiff -l 10 -r 0.01 data/train/output/train.out train.out',
        ],
    },
    '183.equake' => {
        'program' => 'equake',
        'args' => '',
        'input' => 'inp.in',
        'output' => 'inp.out',
        'errout' => 'inp.err',
        'compare' => [
            'specdiff -l 10 -r 1e-05 data/train/output/inp.out inp.out',
        ],
    },
    '187.facerec' => {
        'program' => 'facerec',
        'args' => '',
        'input' => 'train.in',
        'output' => 'train.out',
        'errout' => 'train.err',
        'compare' => [
            'specdiff -l 10 -a 5 -r 0.2 -s 4 data/train/output/hops.out hops.out',
            'specdiff -l 10 -a 2e-07 -r 0.001 -s 0 data/train/output/train.out train.out',
        ],
    },
    '188.ammp' => {
        'program' => 'ammp',
        'args' => '',
        'input' => 'ammp.in',
        'output' => 'ammp.out',
        'errout' => 'ammp.err',
        'compare' => [
            'specdiff -l 10 -a 0.0001 -r 0.003 data/train/output/ammp.out ammp.out',
        ],
    },
    '189.lucas' => {
        'program' => 'lucas',
        'args' => '',
        'input' => 'lucas2.in',
        'output' => 'lucas2.out',
        'errout' => 'lucas2.err',
        'compare' => [
            'specdiff -l 10 data/train/output/lucas2.out lucas2.out',
        ],
    },
    '191.fma3d' => {
        'program' => 'fma3d',
        'args' => '',
        'input' => '',
        'output' => 'fma3d.out',
        'errout' => 'fma3d.err',
        'compare' => [
            'specdiff -l 10 -a 1e-07 -r 0.04 data/train/output/fma3d.out fma3d.out',
        ],
    },
    '200.sixtrack' => {
        'program' => 'sixtrack',
        'args' => '',
        'input' => 'inp.in',
        'output' => 'inp.out',
        'errout' => 'inp.err',
        'compare' => [
            'specdiff -l 10 -a 0.0005 -r 0.0005 data/train/output/inp.out inp.out',
        ],
    },
    '301.apsi' => {
        'program' => 'apsi',
        'args' => '',
        'input' => '',
        'output' => 'apsi.out',
        'errout' => 'apsi.err',
        'compare' => [
            'specdiff -l 10 -a 1e-07 -r 0.01 data/train/output/APO10 APO10',
            'specdiff -l 10 -a 1e-07 -r 0.01 data/train/output/APO11 APO11',
            'specdiff -l 10 -a 1e-07 -r 0.01 data/train/output/APO6 APO6',
            'specdiff -l 10 -a 1e-07 -r 0.01 data/train/output/APO8 APO8',
            'specdiff -l 10 -a 1e-07 -r 0.01 data/train/output/APV APV',
        ],
    },
};


$spec2k{ref} = {
    '164.gzip_input.graphic' => {
        'program' => 'gzip',
        'args' => 'input.graphic 60',
        'input' => '',
        'output' => 'input.graphic.out',
        'errout' => 'input.graphic.err',
        'compare' => [
            'specdiff -l 10 data/ref/output/input.graphic.out input.graphic.out',
        ],
    },
    '164.gzip_input.log' => {
        'program' => 'gzip',
        'args' => 'input.log 60',
        'input' => '',
        'output' => 'input.log.out',
        'errout' => 'input.log.err',
        'compare' => [
            'specdiff -l 10 data/ref/output/input.log.out input.log.out',
        ],
    },
    '164.gzip_input.program' => {
        'program' => 'gzip',
        'args' => 'input.program 60',
        'input' => '',
        'output' => 'input.program.out',
        'errout' => 'input.program.err',
        'compare' => [
            'specdiff -l 10 data/ref/output/input.program.out input.program.out',
        ],
    },
    '164.gzip_input.random' => {
        'program' => 'gzip',
        'args' => 'input.random 60',
        'input' => '',
        'output' => 'input.random.out',
        'errout' => 'input.random.err',
        'compare' => [
            'specdiff -l 10 data/ref/output/input.random.out input.random.out',
        ],
    },
    '164.gzip_input.source' => {
        'program' => 'gzip',
        'args' => 'input.source 60',
        'input' => '',
        'output' => 'input.source.out',
        'errout' => 'input.source.err',
        'compare' => [
            'specdiff -l 10 data/ref/output/input.source.out input.source.out',
        ],
    },
    '175.vpr_place_log' => {
        'program' => 'vpr',
        'args' => 'net.in arch.in place.out dum.out -nodisp -place_only -init_t 5 -exit_t 0.005 -alpha_t 0.9412 -inner_num 2',
        'input' => '',
        'output' => 'place_log.out',
        'errout' => 'place_log.err',
        'compare' => [
            'specdiff -l 10 -r 0.015 data/ref/output/place_log.out place_log.out',
        ],
    },
    '175.vpr_route_log' => {
        'program' => 'vpr',
        'args' => 'net.in arch.in place.in route.out -nodisp -route_only -route_chan_width 15 -pres_fac_mult 2 -acc_fac 1 -first_iter_pres_fac 4 -initial_pres_fac 8',
        'input' => '',
        'output' => 'route_log.out',
        'errout' => 'route_log.err',
        'compare' => [
            'specdiff -l 10 -r 0.05 data/ref/output/costs.out costs.out',
            'specdiff -l 10 -r 0.015 data/ref/output/route.out route.out',
            'specdiff -l 10 -r 0.015 data/ref/output/route_log.out route_log.out',
        ],
    },
    '176.gcc_166' => {
        'program' => 'cc1',
        'args' => '166.i -o 166.s',
        'input' => '',
        'output' => '166.out',
        'errout' => '166.err',
        'compare' => [
            'specdiff -l 10 data/ref/output/166.s 166.s',
        ],
    },
    '176.gcc_expr' => {
        'program' => 'cc1',
        'args' => 'expr.i -o expr.s',
        'input' => '',
        'output' => 'expr.out',
        'errout' => 'expr.err',
        'compare' => [
            'specdiff -l 10 data/ref/output/expr.s expr.s',
        ],
    },
    '176.gcc_integrate' => {
        'program' => 'cc1',
        'args' => 'integrate.i -o integrate.s',
        'input' => '',
        'output' => 'integrate.out',
        'errout' => 'integrate.err',
        'compare' => [
            'specdiff -l 10 data/ref/output/integrate.s integrate.s',
        ],
    },
    '176.gcc_scilab' => {
        'program' => 'cc1',
        'args' => 'scilab.i -o scilab.s',
        'input' => '',
        'output' => 'scilab.out',
        'errout' => 'scilab.err',
        'compare' => [
            'specdiff -l 10 data/ref/output/scilab.s scilab.s',
        ],
    },
    '176.gcc_200' => {
        'program' => 'cc1',
        'args' => '200.i -o 200.s',
        'input' => '',
        'output' => '200.out',
        'errout' => '200.err',
        'compare' => [
            'specdiff -l 10 data/ref/output/200.s 200.s',
        ],
    },
    '181.mcf' => {
        'program' => 'mcf',
        'args' => 'inp.in',
        'input' => '',
        'output' => 'inp.out',
        'errout' => 'inp.err',
        'compare' => [
            'specdiff -l 10 data/ref/output/inp.out inp.out',
            'specdiff -l 10 data/ref/output/mcf.out mcf.out',
        ],
    },
    '186.crafty' => {
        'program' => 'crafty',
        'args' => '',
        'input' => 'crafty.in',
        'output' => 'crafty.out',
        'errout' => 'crafty.err',
        'compare' => [
            'specdiff -l 10 data/ref/output/crafty.out crafty.out',
        ],
    },
    '197.parser' => {
        'program' => 'parser',
        'args' => '2.1.dict -batch',
        'input' => 'ref.in',
        'output' => 'ref.out',
        'errout' => 'ref.err',
        'compare' => [
            'specdiff -l 10 data/ref/output/ref.out ref.out',
        ],
    },
    '252.eon_kajiya_log' => {
        'program' => 'eon',
        'args' => 'chair.control.kajiya chair.camera chair.surfaces chair.kajiya.ppm ppm pixels_out.kajiya',
        'input' => '',
        'output' => 'kajiya_log.out',
        'errout' => 'kajiya_log.err',
        'compare' => [
            'specdiff -l 10 -a 0.005 data/ref/output/pixels_out.kajiya pixels_out.kajiya',
        ],
    },
    '252.eon_cook_log' => {
        'program' => 'eon',
        'args' => 'chair.control.cook chair.camera chair.surfaces chair.cook.ppm ppm pixels_out.cook',
        'input' => '',
        'output' => 'cook_log.out',
        'errout' => 'cook_log.err',
        'compare' => [
            'specdiff -l 10 -a 0.005 data/ref/output/pixels_out.cook pixels_out.cook',
        ],
    },
    '252.eon_rushmeier_log' => {
        'program' => 'eon',
        'args' => 'chair.control.rushmeier chair.camera chair.surfaces chair.rushmeier.ppm ppm pixels_out.rushmeier',
        'input' => '',
        'output' => 'rushmeier_log.out',
        'errout' => 'rushmeier_log.err',
        'compare' => [
            'specdiff -l 10 -a 0.005 data/ref/output/pixels_out.rushmeier pixels_out.rushmeier',
        ],
    },
    '253.perlbmk_makerand' => {
        'program' => 'perlbmk',
        'args' => '-I. -I./lib makerand.pl',
        'input' => '',
        'output' => 'makerand.out',
        'errout' => 'makerand.err',
        'compare' => [
            'specdiff -l 10 data/ref/output/makerand.out makerand.out',
        ],
    },
    '253.perlbmk_850.5.19.18.1500' => {
        'program' => 'perlbmk',
        'args' => '-I./lib splitmail.pl 850 5 19 18 1500',
        'input' => '',
        'output' => '850.5.19.18.1500.out',
        'errout' => '850.5.19.18.1500.err',
        'compare' => [
            'specdiff -l 10 data/ref/output/850.5.19.18.1500.out 850.5.19.18.1500.out',
        ],
    },
    '253.perlbmk_b.3.m.4' => {
        'program' => 'perlbmk',
        'args' => '-I./lib perfect.pl b 3 m 4',
        'input' => '',
        'output' => 'b.3.m.4.out',
        'errout' => 'b.3.m.4.err',
        'compare' => [
            'specdiff -l 10 data/ref/output/b.3.m.4.out b.3.m.4.out',
        ],
    },
    '253.perlbmk_2.550.15.24.23.100' => {
        'program' => 'perlbmk',
        'args' => '-I./lib diffmail.pl 2 550 15 24 23 100',
        'input' => '',
        'output' => '2.550.15.24.23.100.out',
        'errout' => '2.550.15.24.23.100.err',
        'compare' => [
            'specdiff -l 10 data/ref/output/2.550.15.24.23.100.out 2.550.15.24.23.100.out',
        ],
    },
    '253.perlbmk_704.12.26.16.836' => {
        'program' => 'perlbmk',
        'args' => '-I./lib splitmail.pl 704 12 26 16 836',
        'input' => '',
        'output' => '704.12.26.16.836.out',
        'errout' => '704.12.26.16.836.err',
        'compare' => [
            'specdiff -l 10 data/ref/output/704.12.26.16.836.out 704.12.26.16.836.out',
        ],
    },
    '253.perlbmk_535.13.25.24.1091' => {
        'program' => 'perlbmk',
        'args' => '-I./lib splitmail.pl 535 13 25 24 1091',
        'input' => '',
        'output' => '535.13.25.24.1091.out',
        'errout' => '535.13.25.24.1091.err',
        'compare' => [
            'specdiff -l 10 data/ref/output/535.13.25.24.1091.out 535.13.25.24.1091.out',
        ],
    },
    '253.perlbmk_957.12.23.26.1014' => {
        'program' => 'perlbmk',
        'args' => '-I./lib splitmail.pl 957 12 23 26 1014',
        'input' => '',
        'output' => '957.12.23.26.1014.out',
        'errout' => '957.12.23.26.1014.err',
        'compare' => [
            'specdiff -l 10 data/ref/output/957.12.23.26.1014.out 957.12.23.26.1014.out',
        ],
    },
    '254.gap' => {
        'program' => 'gap',
        'args' => '-l ./ -q -m 192M',
        'input' => 'ref.in',
        'output' => 'ref.out',
        'errout' => 'ref.err',
        'compare' => [
            'specdiff -l 10 -r 0.01 data/ref/output/ref.out ref.out',
        ],
    },
    '255.vortex_vortex1.out2' => {
        'program' => 'vortex',
        'args' => 'lendian1.raw',
        'input' => '',
        'output' => 'vortex1.out2',
        'errout' => 'vortex1.err',
        'compare' => [
            'specdiff -l 10 data/ref/output/vortex1.out vortex1.out',
        ],
    },
    '255.vortex_vortex2.out2' => {
        'program' => 'vortex',
        'args' => 'lendian2.raw',
        'input' => '',
        'output' => 'vortex2.out2',
        'errout' => 'vortex2.err',
        'compare' => [
            'specdiff -l 10 data/ref/output/vortex2.out vortex2.out',
        ],
    },
    '255.vortex_vortex3.out2' => {
        'program' => 'vortex',
        'args' => 'lendian3.raw',
        'input' => '',
        'output' => 'vortex3.out2',
        'errout' => 'vortex3.err',
        'compare' => [
            'specdiff -l 10 data/ref/output/vortex3.out vortex3.out',
        ],
    },
    '256.bzip2_input.source' => {
        'program' => 'bzip2',
        'args' => 'input.source 58',
        'input' => '',
        'output' => 'input.source.out',
        'errout' => 'input.source.err',
        'compare' => [
            'specdiff -l 10 data/ref/output/input.source.out input.source.out',
        ],
    },
    '256.bzip2_input.graphic' => {
        'program' => 'bzip2',
        'args' => 'input.graphic 58',
        'input' => '',
        'output' => 'input.graphic.out',
        'errout' => 'input.graphic.err',
        'compare' => [
            'specdiff -l 10 data/ref/output/input.graphic.out input.graphic.out',
        ],
    },
    '256.bzip2_input.program' => {
        'program' => 'bzip2',
        'args' => 'input.program 58',
        'input' => '',
        'output' => 'input.program.out',
        'errout' => 'input.program.err',
        'compare' => [
            'specdiff -l 10 data/ref/output/input.program.out input.program.out',
        ],
    },
    '300.twolf' => {
        'program' => 'twolf',
        'args' => 'ref',
        'input' => '',
        'output' => 'ref.stdout',
        'errout' => 'ref.err',
        'compare' => [
            'specdiff -l 10 -o data/ref/output/ref.out ref.out',
            'specdiff -l 10 -o data/ref/output/ref.pin ref.pin',
            'specdiff -l 10 -o data/ref/output/ref.pl1 ref.pl1',
            'specdiff -l 10 -o data/ref/output/ref.pl2 ref.pl2',
            'specdiff -l 10 -o data/ref/output/ref.sav ref.sav',
            'specdiff -l 10 -o data/ref/output/ref.stdout ref.stdout',
            'specdiff -l 10 -o data/ref/output/ref.sv2 ref.sv2',
            'specdiff -l 10 -o data/ref/output/ref.twf ref.twf',
        ],
    },
    '168.wupwise' => {
        'program' => 'wupwise',
        'args' => '',
        'input' => '',
        'output' => 'wupwise.out',
        'errout' => 'wupwise.err',
        'compare' => [
            'specdiff -l 10 -r 0.005 data/ref/output/te.out te.out',
            'specdiff -l 10 -r 0.001 data/ref/output/wupwise.out wupwise.out',
        ],
    },
    '171.swim' => {
        'program' => 'swim',
        'args' => '',
        'input' => 'swim.in',
        'output' => 'swim.out',
        'errout' => 'swim.err',
        'compare' => [
            'specdiff -l 10 -a 1e-06 -r 0.2 data/ref/output/SWIM7 SWIM7',
            'specdiff -l 10 -a 1e-06 -r 0.001 data/ref/output/swim.out swim.out',
        ],
    },
    '172.mgrid' => {
        'program' => 'mgrid',
        'args' => '',
        'input' => 'mgrid.in',
        'output' => 'mgrid.out',
        'errout' => 'mgrid.err',
        'compare' => [
            'specdiff -l 10 -a 1e-12 -r 0.001 data/ref/output/mgrid.out mgrid.out',
        ],
    },
    '173.applu' => {
        'program' => 'applu',
        'args' => '',
        'input' => 'applu.in',
        'output' => 'applu.out',
        'errout' => 'applu.err',
        'compare' => [
            'specdiff -l 10 -r 0.0001 data/ref/output/applu.out applu.out',
        ],
    },
    '177.mesa' => {
        'program' => 'mesa',
        'args' => '-frames 1000 -meshfile mesa.in -ppmfile mesa.ppm',
        'input' => '',
        'output' => '',
        'errout' => '',
        'compare' => [
            'specdiff -l 10 -a 6 -o -s 6 data/ref/output/mesa.log mesa.log',
            'specdiff -l 10 -a 6 -o -s 6 data/ref/output/mesa.ppm mesa.ppm',
        ],
    },
    '178.galgel' => {
        'program' => 'galgel',
        'args' => '',
        'input' => 'galgel.in',
        'output' => 'galgel.out',
        'errout' => 'galgel.err',
        'compare' => [
            'specdiff -l 10 -a 2e-08 -r 0.01 data/ref/output/galgel.out galgel.out',
        ],
    },
    '179.art_ref.1' => {
        'program' => 'art',
        'args' => '-scanfile c756hel.in -trainfile1 a10.img -trainfile2 hc.img -stride 2 -startx 110 -starty 200 -endx 160 -endy 240 -objects 10',
        'input' => '',
        'output' => 'ref.1.out',
        'errout' => 'ref.1.err',
        'compare' => [
            'specdiff -l 10 -r 0.01 data/ref/output/ref.1.out ref.1.out',
        ],
    },
    '179.art_ref.2' => {
        'program' => 'art',
        'args' => '-scanfile c756hel.in -trainfile1 a10.img -trainfile2 hc.img -stride 2 -startx 470 -starty 140 -endx 520 -endy 180 -objects 10',
        'input' => '',
        'output' => 'ref.2.out',
        'errout' => 'ref.2.err',
        'compare' => [
            'specdiff -l 10 -r 0.01 data/ref/output/ref.2.out ref.2.out',
        ],
    },
    '183.equake' => {
        'program' => 'equake',
        'args' => '',
        'input' => 'inp.in',
        'output' => 'inp.out',
        'errout' => 'inp.err',
        'compare' => [
            'specdiff -l 10 -r 1e-05 data/ref/output/inp.out inp.out',
        ],
    },
    '187.facerec' => {
        'program' => 'facerec',
        'args' => '',
        'input' => 'ref.in',
        'output' => 'ref.out',
        'errout' => 'ref.err',
        'compare' => [
            'specdiff -l 10 -a 5 -r 0.2 -s 4 data/ref/output/hops.out hops.out',
            'specdiff -l 10 -a 2e-07 -r 0.001 -s 0 data/ref/output/ref.out ref.out',
        ],
    },
    '188.ammp' => {
        'program' => 'ammp',
        'args' => '',
        'input' => 'ammp.in',
        'output' => 'ammp.out',
        'errout' => 'ammp.err',
        'compare' => [
            'specdiff -l 10 -a 0.0001 -r 0.003 data/ref/output/ammp.out ammp.out',
        ],
    },
    '189.lucas' => {
        'program' => 'lucas',
        'args' => '',
        'input' => 'lucas2.in',
        'output' => 'lucas2.out',
        'errout' => 'lucas2.err',
        'compare' => [
            'specdiff -l 10 data/ref/output/lucas2.out lucas2.out',
        ],
    },
    '191.fma3d' => {
        'program' => 'fma3d',
        'args' => '',
        'input' => '',
        'output' => 'fma3d.out',
        'errout' => 'fma3d.err',
        'compare' => [
            'specdiff -l 10 -a 1e-07 -r 0.04 data/ref/output/fma3d.out fma3d.out',
        ],
    },
    '200.sixtrack' => {
        'program' => 'sixtrack',
        'args' => '',
        'input' => 'inp.in',
        'output' => 'inp.out',
        'errout' => 'inp.err',
        'compare' => [
            'specdiff -l 10 -a 0.0005 -r 0.0005 data/ref/output/inp.out inp.out',
        ],
    },
    '301.apsi' => {
        'program' => 'apsi',
        'args' => '',
        'input' => '',
        'output' => 'apsi.out',
        'errout' => 'apsi.err',
        'compare' => [
            'specdiff -l 10 -a 1e-07 -r 0.01 data/ref/output/APO10 APO10',
            'specdiff -l 10 -a 1e-07 -r 0.01 data/ref/output/APO11 APO11',
            'specdiff -l 10 -a 1e-07 -r 0.01 data/ref/output/APO6 APO6',
            'specdiff -l 10 -a 1e-07 -r 0.01 data/ref/output/APO8 APO8',
            'specdiff -l 10 -a 1e-07 -r 0.01 data/ref/output/APV APV',
        ],
    },
};

1;
