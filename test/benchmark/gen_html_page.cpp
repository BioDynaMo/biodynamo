#include <fstream>

using namespace std;

int main()
{
    ofstream my_web;
    my_web.open("benchmark/my_web.html");
    my_web << "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\"";
    my_web << "<html>";
    my_web << "<head>";
    my_web << "<meta charset=\"utf-8\">";
    my_web << "<title>my_web</title>";
    my_web << "</head>";
    my_web << "<body>";
    my_web << "<h1>Benchmark</h1>";
    my_web << "<p>One of our goals is to produce highly efficient code  as from CPU, also memory usage.";
    my_web << "Because of that we constantly evaluate the quality of the biodynamo code. Our <a href=\"https://en.wikipedia.org/wiki/Benchmark_(computing)\">benchmark</a> code evauates biodynamo library based on number of demo use-cases, from the CPU and memory usage.";
    my_web << "<h2>CPU time and memory</h2>";
    my_web << "<p>To evaluate the performance of BioDynaMo, we use the benchmark for each demo of the biodynamo. We ran the demos Soma Clustering and Tumor Concepta and compare the values. In the graphics, the bule plots is for the CPU Time and the red plots is for the Memory Usage. The x axis is for the different version of biodynamo.<p>";
    my_web << "<div class=\"figure\">";
    my_web << "<img src=\"SomaClustering0.png\"";
    my_web << "alt=\"Demo Soma Clustering without visualisation\"";
    my_web << "title=\"Demo Soma Clustering without visualisation\">";
    my_web << "<p>Demo Soma Clustering without visualisation<p>";
    my_web << "<img src=\"SomaClustering1.png\"";
    my_web << "alt=\"Demo Soma Clustering with visualisation\"";
    my_web << "title=\"Demo Soma Clustering with visualisation\">";
    my_web << "<p>Demo Soma Clustering with visualisation<p>";
    my_web << "<img src=\"TumorConcept0.png\"";
    my_web << "alt=\"Demo Tumor Concept without visualisation\"";
    my_web << "title=\"Demo Tumor Concept without visualisation\">";
    my_web << "<p>Demo Tumor Concept without visualisation<p>";
    my_web << "<img src=\"TumorConcept1.png\"";
    my_web << "alt=\"Demo Tumor Concept with visualisation\"";
    my_web << "title=\"Demo Tumor Concept with visualisation\">";
    my_web << "<p>Demo Tumor Concept with visualisation<p>";
    my_web << "</div>";
    my_web << "</body>";
    my_web << "</html>";
    my_web.close();
    return 0;
}