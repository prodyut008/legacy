=begin
    analyze2.rb is part of the GeneRic ASsembler (GRAS)
    Copyright (C) 2008  Daniel Otte (daniel.otte@rub.de)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
=end

require 'load_from_xml.rb'
require 'instructionset.rb'

def load(fname)
  iset=load_instructionset_from_xml(fname)
end

def analyze(iset)
  if iset.class==String
    iset = load(iset)
  end
  if iset.is_simple
    puts "Instructionset is simple"
  else
    puts "Instructionset is not simple, violating instructions are:"
    iset.simple_violator.each { |i| puts "  "+i}
  end
  iset.parameters.each { |p| p.generate_c_decoder(STDOUT)}
end

irb