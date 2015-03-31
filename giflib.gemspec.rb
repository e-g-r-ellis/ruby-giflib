Gem::Specification.new do |spec|
	spec.name = 'giflib'
	spec.version = '0.1'
	spec.summary = 'Summary'
	spec.description = 'Description'
	spec.email = 'ignore this'
	spec.homepage = 'http://myapp.com'
	spec.author = 'me'
#	spec.bindir = 'bin'
#	spec.executable = 'exec.rb'
	spec.files = Dir['lib/**/*.rb'] + Dir['ext/**/*.*'] + Dir['ext/**/*']
	spec.platform = Gem::Platform::RUBY
	spec.require_paths = [ 'ext' ]
	spec.extensions = Dir['ext/**/extconf.rb']
end

