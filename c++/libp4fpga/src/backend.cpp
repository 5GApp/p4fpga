
#include "backend.h"
#include <boost/filesystem.hpp>
#include <fstream>

#include "lib/error.h"
#include "lib/nullstream.h"
#include "frontends/p4/evaluator/evaluator.h"
#include "fprogram.h"
#include "ftest.h"
#include "ftype.h"
#include "bsvprogram.h"


namespace FPGA {
void run_fpga_backend(const Options& options, const IR::ToplevelBlock* toplevel,
                      P4::ReferenceMap* refMap, const P4::TypeMap* typeMap) {
    if (toplevel == nullptr)
        return;

    auto main = toplevel->getMain();
    if (main == nullptr) {
        ::error("Could not locate top-level block; is there a %1% module?", IR::P4Program::main);
        return;
    }

    FPGATypeFactory::createFactory(typeMap);

    FPGAProgram fpgaprog(toplevel->getProgram(), refMap, typeMap, toplevel);
    if (!fpgaprog.build())
        return;
    if (options.outputFile.isNullOrEmpty())
        return;

    boost::filesystem::path dir(options.outputFile);
    boost::filesystem::create_directory(dir);

    // TODO(rjs): start here to change to program
    BSVProgram bsv;
    fpgaprog.emit(bsv);

    boost::filesystem::path parserFile("ParserGenerated.bsv");
    boost::filesystem::path parserPath = dir / parserFile;

    boost::filesystem::path structFile("StructGenerated.bsv");
    boost::filesystem::path structPath = dir / structFile;

    boost::filesystem::path deparserFile("DeparserGenerated.bsv");
    boost::filesystem::path deparserPath = dir / deparserFile;

    // ControlGenerated.bsv

    Graph graph;
    fpgaprog.generateGraph(graph);

    boost::filesystem::path graphFile("graph.dot");
    boost::filesystem::path graphPath = dir / graphFile;

    std::ofstream(parserPath.native())   <<  bsv.getParserBuilder().toString();
    std::ofstream(deparserPath.native()) <<  bsv.getDeparserBuilder().toString();
    std::ofstream(structPath.native())   <<  bsv.getStructBuilder().toString();

    std::ofstream(graphPath.native()) << graph.getGraphBuilder().toString();
}

}  // namespace FPGA
